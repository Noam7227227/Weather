# ESP32 Weather Station

An ESP32-based weather station project built using PlatformIO and the Arduino framework. It measures ambient temperature and humidity in real-time, outputs telemetry to the Serial Monitor, tracks Min/Max statistics, and features interactive page navigation via a hardware push button on an OLED display.

---

## 🌟 Project Overview & Evolution

### Phase 1: Basic Weather Station
The initial version of the weather station focused on core telemetry collection:
* **DHT11 Sensor Integration**: Reads ambient temperature (°C) and relative humidity (%) every 2 seconds.
* **Serial Telemetry**: Streams real-time readings to the Serial Monitor at `115200` baud rate.
* **Error Detection**: Handles read timeouts or disconnected sensor pins with diagnostic Serial logs.

### Phase 2: OLED Display Addition
The project was upgraded to function standalone without requiring a USB serial connection:
* **SSD1306 128x64 OLED Integration**: Displays real-time temperature, humidity, and status messages on a 0.96" monochrome screen.
* **Dynamic I2C Bus Auto-Scanning**: Automatically scans I2C pins at startup (checking primary pins `SDA=27, SCL=33`, then falling back to standard ESP32 pins `SDA=21, SCL=22`) to detect the display address dynamically.
* **Custom Bitmap Weather Icons**: Features PROGMEM-stored 12x12 pixel graphics:
  * ☀️ **Sun Icon**: Rendered alongside `"It's hot in here!"` when temperature $\ge 25^\circ\text{C}$.
  * ❄️ **Snowflake Icon**: Rendered alongside `"It's cold in here!"` when temperature $< 25^\circ\text{C}$.
* **On-Screen Diagnostics**: Displays startup initialization progress and hardware error alerts directly on the screen if the DHT sensor fails.

### Phase 3: Hardware Button & Dual-Page UI Navigation
The station now features physical interaction and historical statistics tracking:
* **Push Button Integration (GPIO 25)**: Configured with `INPUT_PULLUP` to switch display screens instantly on button press.
* **Non-Blocking `millis()` Loop**: Converted sensor reading and button polling to asynchronous `millis()` timing, eliminating delay lag and enabling instant button responsiveness.
* **Dual-Page Display System**:
  * **Page 1 (`[Page 1/2: Current]`)**: Shows live temperature, humidity, and Hot/Cold bitmap icons.
  * **Page 2 (`[Page 2/2: Min/Max]`)**: Tracks and displays historical minimum (`minT`) and maximum (`maxT`) temperatures recorded since power-on.
* **Enhanced Serial Logging**: Reports live readings along with running Min/Max temperature bounds.

---

## Hardware Bill of Materials & Wiring

### Bill of Materials (BOM)
| Component | Quantity | Notes |
| :--- | :--- | :--- |
| **ESP32 Development Board** | 1 | NodeMCU-32S / ESP32-WROOM-32 |
| **DHT11 Sensor Module** | 1 | Temperature & Humidity Sensor |
| **SSD1306 OLED Display** | 1 | 0.96" 128x64 I2C Screen |
| **Tactile Push Button** | 1 | Connected to GPIO 25 & GND |
| **Jumper Wires & Breadboard** | - | For circuit connections |

### Pinout Mapping

```
               +-----------------------+
               |     ESP32 DevBoard    |
               +-----------------------+
                |     |     |     |   |   |
         3.3V --+     |     |     |   |   +-- 5V/3.3V
          GND --------+-----+-----+---+---+-- GND
       GPIO13 --------------+     |   |
       GPIO25 (Button) -----------+   |
       GPIO27 (SDA) ------------------+
       GPIO33 (SCL) ----------------------+
```

| ESP32 Pin | Component | Component Pin | Function |
| :--- | :--- | :--- | :--- |
| **GPIO 13** | DHT11 Sensor | Data / OUT | Temperature & Humidity Signal |
| **GPIO 25** | Push Button | Terminal 1 (Terminal 2 to GND) | Page Toggle Input (Internal Pull-Up) |
| **GPIO 27** | SSD1306 OLED | SDA | I2C Data Line |
| **GPIO 33** | SSD1306 OLED | SCL | I2C Clock Line |
| **3.3V / 5V** | DHT11 & OLED | VCC | Power Supply |
| **GND** | DHT11, OLED, Button | GND | Common Ground |

---

## Getting Started

### 1. Prerequisites
* Install [VS Code](https://code.visualstudio.com/) with the [PlatformIO IDE Extension](https://platformio.org/).
* Connect your ESP32 board via USB.

### 2. Build & Flash
Open terminal in the project directory:

```bash
# Build the project
platformio run

# Flash firmware to connected ESP32
platformio run --target upload

# Open Serial Monitor
platformio device monitor
```

---
