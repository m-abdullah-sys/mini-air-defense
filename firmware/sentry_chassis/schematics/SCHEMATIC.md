# Sentry Chassis Schematic

## Overview
This document contains the electrical schematic for the sentry chassis unit of the mini air defense system with complete thermal targeting and coilgun firing control.

## Components

### Main Microcontroller
- **Arduino Board**: Arduino Uno/Nano
- **Operating Voltage**: 5V
- **Clock Speed**: 16 MHz

### Sensing Systems
- **Thermal Camera**: MLX90618 Infrared Thermopile Sensor
  - **Interface**: I2C (SDA: A4, SCL: A5)
  - **I2C Address**: 0x5A
  - **Operating Range**: -20°C to +85°C
  - **Accuracy**: ±2°C

### Actuation Systems
- **Pan Servo Motor**: Standard servo
  - **Pin**: D8
  - **Range**: 0-180°
  - **Control**: PWM @ 50Hz

- **Tilt Servo Motor**: Standard servo
  - **Pin**: D9
  - **Range**: 0-180°
  - **Control**: PWM @ 50Hz

- **Drive Motors**: 4x DC motors (arcade mixing)
  - **Motor Left Forward**: Pin D2
  - **Motor Left Backward**: Pin D3
  - **Motor Right Forward**: Pin D4
  - **Motor Right Backward**: Pin D5
  - **Motor Left Speed (PWM)**: Pin D6
  - **Motor Right Speed (PWM)**: Pin D7
  - **Voltage**: 6-12V
  - **Current**: ~1A per motor

### Coilgun System (450V Capacitor)

#### Charging Circuit
- **P817 Optocoupler**: Isolated gate control
  - **Input Pin**: D10 (5V logic)
  - **Purpose**: Safe isolation between logic and high-voltage charging
  - **Output**: Triggers charging circuit

- **70TPS12 MOSFET**: High-voltage switching
  - **Gate**: Driven by P817 output
  - **Source**: Connected to 450V capacitor
  - **Drain**: Coil gun charging circuit
  - **Current Rating**: 12A
  - **Voltage Rating**: 700V

#### Capacitor
- **Voltage**: 450V
- **Charging Time**: ~2-3 seconds
- **Charge Monitor Pin**: A0 (analog sense line)

#### Fire Circuit
- **Fire MOSFET Trigger**: Pin D11
- **Fire Duration**: 10ms pulse
- **Discharge**: Through coil gun

### Communication Module
- **RF Receiver**: HW-237 Module
- **Interface**: Serial UART (9600 baud)
- **RX Pin**: D0
- **TX Pin**: D1

### Status Indicators
- **RGB LED**: Mode indication
  - **Red Pin**: D12
  - **Green Pin**: D13
  - **Blue Pin**: A1

- **Buzzer**: Audio feedback
  - **Pin**: A2
  - **Operating Voltage**: 5V

## Pin Mapping

| Component | Pin | Type | Voltage | Description |
|-----------|-----|------|---------|-------------|
| Motor Left Fwd | 2 | Digital Out | 5V | Direction control |
| Motor Left Bwd | 3 | Digital Out | 5V | Direction control |
| Motor Right Fwd | 4 | Digital Out | 5V | Direction control |
| Motor Right Bwd | 5 | Digital Out | 5V | Direction control |
| Motor Left Speed | 6 | PWM Out | 5V | Speed control (0-255) |
| Motor Right Speed | 7 | PWM Out | 5V | Speed control (0-255) |
| Pan Servo | 8 | PWM Out | 5V | Pan control |
| Tilt Servo | 9 | PWM Out | 5V | Tilt control |
| Charge Enable | 10 | Digital Out | 5V | Capacitor charging |
| Fire Trigger | 11 | Digital Out | 5V | Coilgun fire pulse |
| LED Red | 12 | Digital Out | 5V | Status LED |
| LED Green | 13 | Digital Out | 5V | Status LED |
| Thermal SDA | A4 | I2C | 5V | I2C data line |
| Thermal SCL | A5 | I2C | 5V | I2C clock line |
| Charge Sense | A0 | Analog In | 0-5V | Capacitor voltage monitor |
| Buzzer | A2 | Digital Out | 5V | Alert signal |
| LED Blue | A1 | Digital Out | 5V | Status LED |
| RF RX | 0 | UART RX | 5V | Serial receive |
| RF TX | 1 | UART TX | 5V | Serial transmit |

## Power Distribution

| Subsystem | Voltage | Current | Notes |
|-----------|---------|---------|-------|
| Arduino Logic | 5V | ~50 mA | Microcontroller & sensors |
| Motors (each) | 6-12V | ~1 A | Total 4A for all motors |
| Servos (each) | 5V | ~150 mA | Total 300mA for both |
| Thermal Sensor | 5V | ~15 mA | MLX90618 |
| RF Module | 5V | ~50 mA | HW-237 receiver |
| Coilgun Charging | 450V | ~2 A during charge | 70TPS12 MOSFET |
| Fire Discharge | 450V | ~50 A (peak) | Through coil |

## Motor Control (Arcade Mixing)

```
Speed_Left = Vertical ± Horizontal
Speed_Right = Vertical ∓ Horizontal

Where:
- Vertical = Joystick Y - 128
- Horizontal = Joystick X - 128
- Range: -255 to +255
```

### Motor Direction Truth Table

| Speed | Direction | Forward Pin | Backward Pin |
|-------|-----------|-------------|--------------|
| > 0 | Forward | HIGH | LOW |
| < 0 | Backward | LOW | HIGH |
| = 0 | Stop | LOW | LOW |

## Wiring Diagram

```
┌─────────────────────────────────────────────────────┐
│          Arduino Sentry Chassis                     │
│                                                     │
│  MOTOR CONTROL:                                    │
│  D2,D3,D4,D5 ──→ Motor Direction                   │
│  D6,D7 ──────→ Motor Speed (PWM)                   │
│                                                     │
│  SERVO CONTROL:                                    │
│  D8 ────→ Pan Servo (0-180°)                      │
│  D9 ────→ Tilt Servo (0-180°)                     │
│                                                     │
│  COILGUN:                                          │
│  D10 ───→ P817 (Charge Enable)                    │
│  D11 ───→ 70TPS12 MOSFET (Fire)                   │
│  A0  ←─── Charge Sense (Feedback)                 │
│                                                     │
│  THERMAL SENSOR (MLX90618):                        │
│  A4 (SDA) ←──→ Thermal I2C                        │
│  A5 (SCL) ←──→ Thermal I2C                        │
│                                                     │
│  STATUS:                                           │
│  D12 ──→ LED Red                                   │
│  D13 ──→ LED Green                                 │
│  A1  ──→ LED Blue                                  │
│  A2  ──→ Buzzer                                    │
│                                                     │
│  COMMUNICATION:                                    │
│  D0 (RX) ←── HW-237 RX                            │
│  D1 (TX) ──→ HW-237 TX                            │
│                                                     │
│  GND ──→ Common Ground                            │
│  5V  ──→ Logic Supply                             │
└─────────────────────────────────────────────────────┘
```

## Thermal Sensor Details (MLX90618)

### I2C Communication
- **Address**: 0x5A (7-bit)
- **Clock Speed**: 400 kHz
- **Data Register**: 0x07 (Object Temperature)

### Temperature Conversion
```
Raw Data: 16-bit value
Temperature (°C) = (Raw * 0.02) - 273.15
```

### Target Detection Threshold
- **Default**: 20°C above ambient
- **Sensitivity**: ±0.02°C per LSB

## Coilgun Firing Sequence

1. **Charging Phase**:
   - Set D10 (Charge Enable) HIGH
   - Monitor A0 (Charge Sense) until target voltage
   - Typical time: 2-3 seconds

2. **Locked Phase**:
   - Joystick aims servos
   - Thermal sensor verifies target
   - Wait for fire command

3. **Fire Phase**:
   - Verify charge level adequate
   - Set D11 (Fire Trigger) HIGH
   - Wait 10ms
   - Set D11 LOW
   - Discharge through coil gun

## Safety Features

- **Auto-Timeout**: 500ms without RF signal
- **Charge Verification**: Must reach threshold before fire
- **Motor Failsafe**: Stops on command loss
- **Servo Centering**: Auto-centers on mode change
- **Current Limiting**: Motor speed through PWM
- **Watchdog**: Serial timeout protection

## Notes

- All motor connections require external power supply (separate from 5V logic)
- Coilgun circuit operates isolated from logic through P817 optocoupler
- RF range: ~100-200m line of sight
- Thermal sensor accuracy improves with emissivity calibration
- Motor mixing ratio can be adjusted in firmware for different turning characteristics
