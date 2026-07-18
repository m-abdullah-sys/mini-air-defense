/*
 * MINI AIR DEFENSE SYSTEM - SENTRY CHASSIS CODE
 * 
 * System Components:
 * - MLX90618 Thermal Sensor (I2C)
 * - HW-237 RF Receiver Module (Serial)
 * - 2x Servo Motors (Pan/Tilt)
 * - 4x DC Motors (Drive wheels)
 * - P817 Optocoupler & 70TPS12 MOSFET (Capacitor charge control)
 * - 450V Capacitor (Coilgun power)
 * - RGB LED Status
 * - Buzzer
 * 
 * Modes:
 * Mode 1: RC Car Only (Green LED)
 * Mode 2: RC Car + Coilgun (Blue LED)
 * Mode 3: Servos + Thermal Sensor + Coilgun (Red LED)
 */

#include <Wire.h>
#include <Servo.h>

// ============ THERMAL SENSOR DEFINES ============
#define MLX90618_ADDRESS 0x5A
#define MLX90618_TA 0x06  // Ambient temperature register
#define MLX90618_TOBJ1 0x07  // Object temperature register

// ============ PIN DEFINITIONS ============
// Motor Pins (RC Car - Mode 1 & 2)
#define MOTOR_LEFT_FORWARD 2
#define MOTOR_LEFT_BACKWARD 3
#define MOTOR_RIGHT_FORWARD 4
#define MOTOR_RIGHT_BACKWARD 5
#define MOTOR_LEFT_SPEED 6
#define MOTOR_RIGHT_SPEED 7

// Servo Pins (Mode 3)
#define SERVO_PAN 8
#define SERVO_TILT 9

// Coilgun Control Pins
#define CHARGE_ENABLE 10     // P817 Optocoupler input
#define FIRE_MOSFET 11       // 70TPS12 MOSFET trigger
#define CHARGE_INDICATOR A0  // Capacitor charge sense

// Status LED Pins
#define LED_RED 12
#define LED_GREEN 13
#define LED_BLUE A1

// RF Module
#define RF_RX_PIN 0    // RX pin
#define RF_TX_PIN 1    // TX pin

// Buzzer
#define BUZZER_PIN A2

// ============ GLOBAL VARIABLES ============
Servo panServo;
Servo tiltServo;

byte currentMode = 1;  // 1: RC Only, 2: RC+Coilgun, 3: Servo+Thermal+Coilgun
byte fireButton = 0;

// RF Command Structure
struct RFCommand {
  byte mode;        // 0x01-0x03
  byte joystickX;   // 0-255
  byte joystickY;   // 0-255
  byte fireCmd;     // 0x00 or 0x01
};

RFCommand command;
unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 500;  // 500ms timeout

// Servo positions
byte panAngle = 90;
byte tiltAngle = 90;

// ============ SETUP ============
void setup() {
  // Serial communication with RF module
  Serial.begin(9600);
  
  // Initialize I2C for thermal sensor
  Wire.begin();
  
  // Motor pins
  pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(MOTOR_LEFT_BACKWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_BACKWARD, OUTPUT);
  pinMode(MOTOR_LEFT_SPEED, OUTPUT);
  pinMode(MOTOR_RIGHT_SPEED, OUTPUT);
  
  // Servo pins
  pinMode(SERVO_PAN, OUTPUT);
  pinMode(SERVO_TILT, OUTPUT);
  
  // Coilgun pins
  pinMode(CHARGE_ENABLE, OUTPUT);
  pinMode(FIRE_MOSFET, OUTPUT);
  pinMode(CHARGE_INDICATOR, INPUT);
  
  // LED pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initialize servos
  panServo.attach(SERVO_PAN);
  tiltServo.attach(SERVO_TILT);
  centerServos();
  
  // Safety: Disable everything initially
  digitalWrite(CHARGE_ENABLE, LOW);
  digitalWrite(FIRE_MOSFET, LOW);
  stopMotors();
  setLED(0, 1, 0);  // Green LED on boot
  
  delay(1000);
  setLED(0, 0, 0);  // All off
}

// ============ MAIN LOOP ============
void loop() {
  // Check for incoming RF commands
  if (Serial.available() >= sizeof(RFCommand)) {
    byte data[sizeof(RFCommand)];
    for (int i = 0; i < sizeof(RFCommand); i++) {
      data[i] = Serial.read();
    }
    memcpy(&command, data, sizeof(RFCommand));
    lastCommandTime = millis();
  }
  
  // Check for command timeout
  if (millis() - lastCommandTime > COMMAND_TIMEOUT) {
    stopMotors();
    digitalWrite(CHARGE_ENABLE, LOW);
    return;
  }
  
  // Handle mode change
  if (command.mode != currentMode) {
    currentMode = command.mode;
    handleModeChange();
  }
  
  // Execute mode logic
  switch (currentMode) {
    case 1:
      modeRCOnly();
      break;
    case 2:
      modeRCCoilgun();
      break;
    case 3:
      modeServoThermalCoilgun();
      break;
  }
}

// ============ MODE FUNCTIONS ============

void modeRCOnly() {
  // Green LED
  setLED(0, 1, 0);
  
  // Process joystick for motor control
  controlMotors(command.joystickX, command.joystickY);
  
  // Disable coilgun and servos
  digitalWrite(CHARGE_ENABLE, LOW);
  panServo.detach();
  tiltServo.detach();
}

void modeRCCoilgun() {
  // Blue LED
  setLED(0, 0, 1);
  
  // Process joystick for motor control
  controlMotors(command.joystickX, command.joystickY);
  
  // Charge capacitor
  digitalWrite(CHARGE_ENABLE, HIGH);
  
  // Check fire command (button 4)
  if (command.fireCmd == 1) {
    fireCoilgun();
    delay(100);  // Debounce
    command.fireCmd = 0;
  }
  
  // Servos not used in this mode
  panServo.detach();
  tiltServo.detach();
}

void modeServoThermalCoilgun() {
  // Red LED
  setLED(1, 0, 0);
  
  // Motors disabled - RC car not controlled
  stopMotors();
  
  // Reattach servos if detached
  if (!panServo.attached()) {
    panServo.attach(SERVO_PAN);
  }
  if (!tiltServo.attached()) {
    tiltServo.attach(SERVO_TILT);
  }
  
  // Control servos with joystick
  // Joystick X controls pan (horizontal)
  // Joystick Y controls tilt (vertical)
  panAngle = map(command.joystickX, 0, 255, 0, 180);
  tiltAngle = map(command.joystickY, 0, 255, 0, 180);
  
  panServo.write(panAngle);
  tiltServo.write(tiltAngle);
  
  // Read thermal sensor
  float objectTemp = readThermalSensor();
  
  // Charge capacitor
  digitalWrite(CHARGE_ENABLE, HIGH);
  
  // Check fire command (button 4)
  if (command.fireCmd == 1) {
    // Simple target detection: if temperature > threshold
    if (objectTemp > 20.0) {  // Adjust threshold as needed
      fireCoilgun();
      beep(2);  // Two beeps for successful lock
    } else {
      beep(1);  // One beep for no target
    }
    delay(100);  // Debounce
    command.fireCmd = 0;
  }
}

// ============ MOTOR CONTROL ============

void controlMotors(byte joystickX, byte joystickY) {
  // Center is around 127-128
  // Forward: Y > 150
  // Backward: Y < 100
  // Left: X < 100
  // Right: X > 150
  
  int speedLeft = 0, speedRight = 0;
  byte dirLeftF = 0, dirLeftB = 0, dirRightF = 0, dirRightB = 0;
  
  // Calculate forward/backward
  int verticalSpeed = (int)joystickY - 128;  // -128 to 127
  
  // Calculate left/right
  int horizontalSpeed = (int)joystickX - 128;  // -128 to 127
  
  // Mix joystick input
  speedLeft = verticalSpeed + horizontalSpeed;
  speedRight = verticalSpeed - horizontalSpeed;
  
  // Constrain speeds
  speedLeft = constrain(speedLeft, -255, 255);
  speedRight = constrain(speedRight, -255, 255);
  
  // Set direction and speed for left motor
  if (speedLeft > 0) {
    dirLeftF = 1;
    dirLeftB = 0;
    analogWrite(MOTOR_LEFT_SPEED, speedLeft);
  } else if (speedLeft < 0) {
    dirLeftF = 0;
    dirLeftB = 1;
    analogWrite(MOTOR_LEFT_SPEED, abs(speedLeft));
  } else {
    dirLeftF = 0;
    dirLeftB = 0;
    analogWrite(MOTOR_LEFT_SPEED, 0);
  }
  
  // Set direction and speed for right motor
  if (speedRight > 0) {
    dirRightF = 1;
    dirRightB = 0;
    analogWrite(MOTOR_RIGHT_SPEED, speedRight);
  } else if (speedRight < 0) {
    dirRightF = 0;
    dirRightB = 1;
    analogWrite(MOTOR_RIGHT_SPEED, abs(speedRight));
  } else {
    dirRightF = 0;
    dirRightB = 0;
    analogWrite(MOTOR_RIGHT_SPEED, 0);
  }
  
  // Write direction pins
  digitalWrite(MOTOR_LEFT_FORWARD, dirLeftF);
  digitalWrite(MOTOR_LEFT_BACKWARD, dirLeftB);
  digitalWrite(MOTOR_RIGHT_FORWARD, dirRightF);
  digitalWrite(MOTOR_RIGHT_BACKWARD, dirRightB);
}

void stopMotors() {
  digitalWrite(MOTOR_LEFT_FORWARD, LOW);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
  analogWrite(MOTOR_LEFT_SPEED, 0);
  analogWrite(MOTOR_RIGHT_SPEED, 0);
}

// ============ COILGUN CONTROL ============

void fireCoilgun() {
  // Check if capacitor is charged
  int chargeLevel = analogRead(CHARGE_INDICATOR);
  
  if (chargeLevel > 200) {  // Adjust threshold based on your circuit
    // Trigger fire MOSFET
    digitalWrite(FIRE_MOSFET, HIGH);
    delay(10);  // Pulse duration
    digitalWrite(FIRE_MOSFET, LOW);
    
    beep(3);  // Three beeps for fire
  } else {
    beep(1);  // One beep for insufficient charge
  }
}

// ============ THERMAL SENSOR ============

float readThermalSensor() {
  // Read object temperature from MLX90618
  Wire.beginTransmission(MLX90618_ADDRESS);
  Wire.write(MLX90618_TOBJ1);
  Wire.endTransmission(false);
  
  Wire.requestFrom(MLX90618_ADDRESS, 2);
  
  if (Wire.available() == 2) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    
    // MLX90618 uses 16-bit data with specific conversion
    int rawTemp = (msb << 8) | lsb;
    float temperature = (rawTemp * 0.02) - 273.15;  // Convert to Celsius
    
    return temperature;
  }
  
  return 0.0;  // Return 0 if read fails
}

// ============ SERVO CONTROL ============

void centerServos() {
  if (panServo.attached()) {
    panServo.write(90);
  }
  if (tiltServo.attached()) {
    tiltServo.write(90);
  }
}

// ============ LED CONTROL ============

void setLED(byte red, byte green, byte blue) {
  digitalWrite(LED_RED, red ? HIGH : LOW);
  digitalWrite(LED_GREEN, green ? HIGH : LOW);
  digitalWrite(LED_BLUE, blue ? HIGH : LOW);
}

// ============ BUZZER CONTROL ============

void beep(byte count) {
  for (byte i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

// ============ MODE CHANGE HANDLER ============

void handleModeChange() {
  setLED(0, 0, 0);  // Turn off all LEDs
  delay(200);
  beep(currentMode);  // Beep count indicates mode
  
  switch (currentMode) {
    case 1:
      setLED(0, 1, 0);  // Green for RC Only
      break;
    case 2:
      setLED(0, 0, 1);  // Blue for RC+Coilgun
      break;
    case 3:
      setLED(1, 0, 0);  // Red for Servo+Thermal+Coilgun
      break;
  }
}
