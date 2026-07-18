/*
 * MINI AIR DEFENSE SYSTEM - REMOTE CONTROL CODE
 * 
 * Remote Control Components:
 * - 1x Joystick (Analog X & Y)
 * - 4x Push Buttons (Mode 1-3 + Fire)
 * - HW-237 RF Transmitter Module (Serial)
 * - RGB LED Status Indicator
 * 
 * Button Functions:
 * Button 1: Mode 1 - RC Car Only (Green LED)
 * Button 2: Mode 2 - RC Car + Coilgun (Blue LED)
 * Button 3: Mode 3 - Servos + Thermal + Coilgun (Red LED)
 * Button 4: Fire Coilgun Shot
 */

// ============ PIN DEFINITIONS ============

// Joystick Pins
#define JOYSTICK_X A0
#define JOYSTICK_Y A1

// Button Pins
#define BUTTON_MODE1 2
#define BUTTON_MODE2 3
#define BUTTON_MODE3 4
#define BUTTON_FIRE 5

// Status LED Pins
#define LED_RED 6
#define LED_GREEN 7
#define LED_BLUE 8

// RF Module
#define RF_RX_PIN 0    // RX pin
#define RF_TX_PIN 1    // TX pin

// ============ GLOBAL VARIABLES ============

byte currentMode = 1;  // Default mode: RC Only
byte fireButtonPressed = 0;

// Joystick calibration values
int joystickXCenter = 512;
int joystickYCenter = 512;
const int JOYSTICK_DEADZONE = 50;

// Button state tracking
boolean button1State = false;
boolean button2State = false;
boolean button3State = false;
boolean button4State = false;

boolean button1LastState = false;
boolean button2LastState = false;
boolean button3LastState = false;
boolean button4LastState = false;

// RF Command Structure
struct RFCommand {
  byte mode;        // 0x01-0x03
  byte joystickX;   // 0-255
  byte joystickY;   // 0-255
  byte fireCmd;     // 0x00 or 0x01
};

RFCommand command;
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 50;  // Send every 50ms

// ============ SETUP ============

void setup() {
  // Serial communication with RF module
  Serial.begin(9600);
  
  // Button pins - input with pull-up
  pinMode(BUTTON_MODE1, INPUT_PULLUP);
  pinMode(BUTTON_MODE2, INPUT_PULLUP);
  pinMode(BUTTON_MODE3, INPUT_PULLUP);
  pinMode(BUTTON_FIRE, INPUT_PULLUP);
  
  // LED pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  // Joystick pins (already analog inputs)
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  
  // Initialize
  calibrateJoystick();
  setLED(0, 1, 0);  // Green LED on boot
  delay(500);
  setLED(0, 0, 0);  // LED off
  
  // Initialize RF command
  command.mode = 1;
  command.joystickX = 128;
  command.joystickY = 128;
  command.fireCmd = 0;
}

// ============ MAIN LOOP ============

void loop() {
  // Read all inputs
  readButtons();
  readJoystick();
  
  // Send RF command at fixed interval
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    sendRFCommand();
    lastSendTime = millis();
  }
  
  // Small delay to prevent overwhelming the loop
  delay(10);
}

// ============ BUTTON READING ============

void readButtons() {
  // Read button states (inverted due to pull-up)
  button1State = !digitalRead(BUTTON_MODE1);
  button2State = !digitalRead(BUTTON_MODE2);
  button3State = !digitalRead(BUTTON_MODE3);
  button4State = !digitalRead(BUTTON_FIRE);
  
  // Detect rising edge (button press)
  
  // Mode 1 Button
  if (button1State && !button1LastState) {
    currentMode = 1;
    handleModeChange();
  }
  button1LastState = button1State;
  
  // Mode 2 Button
  if (button2State && !button2LastState) {
    currentMode = 2;
    handleModeChange();
  }
  button2LastState = button2State;
  
  // Mode 3 Button
  if (button3State && !button3LastState) {
    currentMode = 3;
    handleModeChange();
  }
  button3LastState = button3State;
  
  // Fire Button
  if (button4State && !button4LastState) {
    fireButtonPressed = 1;
  } else if (!button4State) {
    fireButtonPressed = 0;
  }
  button4LastState = button4State;
  
  // Update command
  command.mode = currentMode;
  command.fireCmd = fireButtonPressed;
}

// ============ JOYSTICK READING ============

void readJoystick() {
  // Read raw analog values
  int rawX = analogRead(JOYSTICK_X);
  int rawY = analogRead(JOYSTICK_Y);
  
  // Apply deadzone
  if (abs(rawX - joystickXCenter) < JOYSTICK_DEADZONE) {
    rawX = joystickXCenter;
  }
  if (abs(rawY - joystickYCenter) < JOYSTICK_DEADZONE) {
    rawY = joystickYCenter;
  }
  
  // Map from 0-1023 to 0-255
  byte mappedX = map(rawX, 0, 1023, 0, 255);
  byte mappedY = map(rawY, 0, 1023, 0, 255);
  
  command.joystickX = mappedX;
  command.joystickY = mappedY;
}

// ============ RF TRANSMISSION ============

void sendRFCommand() {
  // Send command structure over RF module
  byte data[sizeof(RFCommand)];
  memcpy(data, &command, sizeof(RFCommand));
  
  // Send each byte
  for (int i = 0; i < sizeof(RFCommand); i++) {
    Serial.write(data[i]);
  }
  
  // Optional: Add simple checksum
  byte checksum = 0;
  for (int i = 0; i < sizeof(RFCommand); i++) {
    checksum += data[i];
  }
  Serial.write(checksum);
}

// ============ LED CONTROL ============

void setLED(byte red, byte green, byte blue) {
  digitalWrite(LED_RED, red ? HIGH : LOW);
  digitalWrite(LED_GREEN, green ? HIGH : LOW);
  digitalWrite(LED_BLUE, blue ? HIGH : LOW);
}

void blinkLED(byte red, byte green, byte blue, int count) {
  for (int i = 0; i < count; i++) {
    setLED(red, green, blue);
    delay(150);
    setLED(0, 0, 0);
    delay(150);
  }
}

// ============ JOYSTICK CALIBRATION ============

void calibrateJoystick() {
  // Read center position
  joystickXCenter = analogRead(JOYSTICK_X);
  joystickYCenter = analogRead(JOYSTICK_Y);
  
  // Blink LED to indicate calibration
  blinkLED(0, 1, 0, 2);
}

// ============ MODE CHANGE HANDLER ============

void handleModeChange() {
  // Turn off LED first
  setLED(0, 0, 0);
  delay(200);
  
  // Set LED color based on mode and blink count
  switch (currentMode) {
    case 1:
      // Green LED - RC Car Only
      setLED(0, 1, 0);
      blinkLED(0, 1, 0, 1);
      break;
      
    case 2:
      // Blue LED - RC Car + Coilgun
      setLED(0, 0, 1);
      blinkLED(0, 0, 1, 2);
      break;
      
    case 3:
      // Red LED - Servos + Thermal + Coilgun
      setLED(1, 0, 0);
      blinkLED(1, 0, 0, 3);
      break;
  }
  
  // Leave LED on to show current mode
  switch (currentMode) {
    case 1:
      setLED(0, 1, 0);  // Green
      break;
    case 2:
      setLED(0, 0, 1);  // Blue
      break;
    case 3:
      setLED(1, 0, 0);  // Red
      break;
  }
}
