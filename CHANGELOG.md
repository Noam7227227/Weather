# Changelog

All notable changes to the ESP32 Weather Station project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.3.0] - 2026-09-16

### Added
- **Wi-Fi Connectivity (`WiFi.h`)** with 10-second non-blocking connection timeout fallback.
- **NTP Time Synchronization (`time.h`)** via `pool.ntp.org` configured for Israel Timezone (UTC+2 / Daylight Savings).
- **3-Page OLED Display Navigation**:
  - **Page 1**: Real-time telemetry, Hot/Cold weather status icons.
  - **Page 2**: Historical Minimum (`minT`) and Maximum (`maxT`) temperature stats.
  - **Page 3**: Live digital clock displaying Date (`DD/MM/YYYY`), Time (`HH:MM:SS`), and Wi-Fi status.
- Automatic 1-second live screen tick update when viewing Page 3 (Clock).

### Changed
- Expanded hardware push button cycling from 2 pages to 3 pages (`0 -> 1 -> 2 -> 0`).
- Updated startup screen to report Wi-Fi connection progress.
- Updated `README.md` with Phase 4 evolution and Wi-Fi configuration instructions.

---

## [1.2.0] - 2026-09-16

### Added
- **Push Button Integration (GPIO 25)** configured with `INPUT_PULLUP` for interactive UI navigation.
- **Dual-Page Display System**:
  - **Page 1**: Live temperature, humidity, and Hot/Cold weather status icons.
  - **Page 2**: Historical Minimum (`minT`) and Maximum (`maxT`) temperature statistics tracking.
- **Non-Blocking Execution Loop**: Replaced blocking `delay(2000)` with asynchronous `millis()` timing for sensor polling and debounced button reading.
- Min/Max temperature bounds outputted to Serial Monitor telemetry.

### Changed
- Refactored OLED rendering logic into dedicated `updateDisplay()` function.
- Updated circuit pinout, ASCII diagrams, and BOM table in `README.md`.

---

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
