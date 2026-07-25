# mini-air-defense

Arduino-based thermal tracking and dual-axis target acquisition prototype.

WARNING — safety & legal notice

This repository contains hardware designs and firmware for a small-scale experimental target detection and engagement system. It is provided for research, education, and testing in controlled, lawful environments only. Do not use, adapt, or deploy these materials for harmful or illegal activities. Always comply with local laws and safety regulations, and operate in a safe test area with appropriate protective measures.

What this repository contains

- firmware/ — microcontroller firmware for remote controls, sentry chassis, and peripherals. See subdirectories for per-board build and upload instructions.
- docs/ — design notes, diagrams, and schematics.
- hardware/ — PCB layouts, connector pinouts, and BOMs (where available).
- tools/ — host-side helper scripts (data logging, telemetry, calibration tools).

Supported hardware (example / reference)

- Microcontroller: Arduino Uno, Nano, or compatible (ATmega328P)
- RF link: nRF24L01+ or other 2.4GHz transceivers (SPI)
- Thermal sensor: MLX90614 or small IR thermopile array
- Motor driver: L298N (differential / tank drive)
- Actuators: 2x hobby servos for pan/tilt and optional pump relay

Getting started

1. Clone the repository
   git clone https://github.com/m-abdullah-sys/mini-air-defense.git

2. Inspect firmware directories
   - firmware/remote_control — transmitter firmware and wiring
   - firmware/sentry_chassis — receiver / sentry firmware and wiring

3. Build & flash
   - Use the Arduino IDE or PlatformIO. Open the appropriate sketch (.ino) for your board and upload. Check per-sketch README files for pin mappings and library dependencies.

4. Test safely
   - Power motors and actuators from appropriate external supplies and ensure a common ground with the microcontroller.
   - Test sensors and radio links with motors/actuators disconnected before enabling movement or firing behaviors.

Contributing

- Open issues for bugs or enhancement requests.
- Create feature branches and open pull requests with clear descriptions and test notes.
- Keep hardware changes documented in docs/ and update BOMs when components change.

License

MIT — see LICENSE file in this repository.

Contact

If you have questions, open an issue or contact the repository owner.
