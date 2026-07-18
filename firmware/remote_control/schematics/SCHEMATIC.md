# Remote Control Schematic

## Overview
This document contains the electrical schematic for the remote control unit of the mini air defense system.

## Components

### Main Microcontroller
- **Arduino Board**: Arduino Uno/Nano
- **Operating Voltage**: 5V

### Input Devices
- **Joystick Module**: Analog input for directional control (X & Y axes)
- **Push Buttons**: 4x Digital input buttons
  - Button 1 (Pin 2): Mode 1 - RC Only
  - Button 2 (Pin 3): Mode 2 - RC + Coilgun
  - Button 3 (Pin 4): Mode 3 - Servo + Thermal + Coilgun
  - Button 4 (Pin 5): Fire Coilgun

### Communication Module
- **RF Transmitter**: HW-237 Module
- **Interface**: Serial UART (9600 baud)
- **Antenna**: 433 MHz antenna

### Status Indicator
- **RGB LED**: Status feedback for current mode
- **Pins**: 
  - Red (Pin 6)
  - Green (Pin 7)
  - Blue (Pin 8)

### Power Supply
- **Battery Type**: 9V or 4x AA cells
- **Voltage**: 5-9V
- **Current Draw**: ~200mA average

## Pin Mapping

| Component | Pin | Type | Description |
|-----------|-----|------|-------------|
| Joystick X | A0 | Analog | Horizontal axis |
| Joystick Y | A1 | Analog | Vertical axis |
| Mode 1 Button | 2 | Digital | Mode select |
| Mode 2 Button | 3 | Digital | Mode select |
| Mode 3 Button | 4 | Digital | Mode select |
| Fire Button | 5 | Digital | Fire coilgun |
| LED Red | 6 | Digital | Status LED |
| LED Green | 7 | Digital | Status LED |
| LED Blue | 8 | Digital | Status LED |
| RF TX RX | 0/1 | UART | Serial communication |

## Wiring Diagram

```
Arduino Remote Control
┌─────────────────────────────────────┐
│         Arduino Uno/Nano            │
│                                     │
│ A0 ←────────── Joystick X          │
│ A1 ←────────── Joystick Y          │
│ D2 ←────────── Mode 1 Button       │
│ D3 ←────────── Mode 2 Button       │
│ D4 ←────────── Mode 3 Button       │
│ D5 ←────────── Fire Button         │
│                                     │
│ D6 ──────────→ LED Red             │
│ D7 ──────────→ LED Green           │
│ D8 ──────────→ LED Blue            │
│                                     │
│ TX/D1 ──────→ HW-237 RX            │
│ RX/D0 ←────── HW-237 TX            │
│                                     │
│ GND ─────────→ Common Ground       │
│ 5V  ─────────→ Joystick VCC        │
└─────────────────────────────────────┘
```

## Power Considerations

- **Total Current Draw**: ~150-200 mA
- **Peak Current**: ~250 mA during RF transmission
- **Voltage Regulation**: Internal regulator for 5V logic
- **Battery**: 9V with 7805 regulator or 4x AA cells directly

## RF Protocol

### Command Structure
```
Byte 0: Mode (1-3)
Byte 1: Joystick X (0-255)
Byte 2: Joystick Y (0-255)
Byte 3: Fire Command (0x00/0x01)
Byte 4: Checksum (sum of bytes 0-3)
```

### Transmission Rate
- **Interval**: 50ms (20 Hz)
- **Baud Rate**: 9600
- **Range**: ~100-200 meters (line of sight)

## Notes

- All buttons use pull-up resistors
- Joystick center position: ~512 on ADC (2.5V)
- Deadzone: ±50 ADC units
- LED color indicates current mode
- RF module operates at 433 MHz
