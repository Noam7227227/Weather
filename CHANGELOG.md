# Changelog

All notable changes to the ESP32 Weather Station project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2026-08-28

### Added
- **SSD1306 OLED Display (128x64)** support over I2C.
- Dynamic I2C scanner (`scanI2C`) checking primary pins (`SDA=27, SCL=33`) with fallback to standard ESP32 pins (`SDA=21, SCL=22`).
- Custom 12x12 PROGMEM bitmap graphics for visual weather condition indicators:
  - Hot Sun bitmap icon for temperatures $\ge 25^\circ\text{C}$.
  - Cold Snowflake bitmap icon for temperatures $< 25^\circ\text{C}$.
- On-screen startup initialization screen and sensor error alerts.

### Changed
- Upgraded weather station from serial-only telemetry to standalone screen visualization.

---

## [1.0.0] - 2026-08-28

### Added
- Initial project release using PlatformIO and the Arduino framework for ESP32.
- **DHT11 Sensor Integration** on GPIO 13 reading ambient temperature (°C) and relative humidity (%).
- 2-second periodic measurement loop.
- Serial telemetry streaming at `115200` baud.
