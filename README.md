# ESP32 Weather Station

An ESP32-based weather station project built using PlatformIO and the Arduino framework. It measures ambient temperature and humidity in real-time, outputs telemetry to the Serial Monitor, tracks Min/Max statistics, connects to Wi-Fi to sync Network Time Protocol (NTP) time, and features interactive 3-page UI navigation via a hardware push button on an OLED display.

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

### Phase 3: Hardware Button & OLED Navigation
Physical interaction and historical statistics tracking:
* **Push Button Integration (GPIO 25)**: Configured with `INPUT_PULLUP` to switch display screens instantly on button press.
* **Non-Blocking `millis()` Loop**: Converted sensor reading and button polling to asynchronous `millis()` timing, eliminating delay lag and enabling instant button responsiveness.
* **Min/Max Tracking**: Keeps running record of lowest and highest recorded temperatures.

### Phase 4: Wi-Fi Connectivity & Live NTP Clock
Networking, secure `.env` secret management, and real-time clock synchronization:
* **`.env` Credentials Management**: Reads Wi-Fi credentials from `.env` via PlatformIO script (`read_env.py`) with template in `.env.example`.
* **Wi-Fi Integration (`WiFi.h`)**: Connects to local 2.4GHz Wi-Fi networks with a non-blocking 10-second connection timeout fallback.
* **NTP Time Sync (`time.h`)**: Synchronizes exact date and time over Network Time Protocol (`pool.ntp.org`).
* **3-Page Display System**:
  * **Page 1 (`[Page 1/3: Current]`)**: Shows live temperature, humidity, and Hot/Cold bitmap icons.
  * **Page 2 (`[Page 2/3: Min/Max]`)**: Displays historical minimum (`minT`) and maximum (`maxT`) temperatures recorded since power-on.
  * **Page 3 (`[Page 3/3: Clock]`)**: Live ticking digital clock displaying Date (`DD/MM/YYYY`), Time (`HH:MM:SS`), and Wi-Fi connection status.

### Phase 5: LittleFS Web Dashboard
* **External HTML Page**: The dashboard markup and styling are in `data/index.html`, separate from the firmware source.
* **LittleFS Hosting**: PlatformIO packages the `data/` directory as a LittleFS image, and the ESP32 serves `/index.html` at its root URL.
* **Live Telemetry**: The page displays temperature, humidity, min/max temperature, and a sensor-error state. It automatically reloads every 5 seconds.
* **OLED Clock**: NTP-synchronized time remains on OLED Page 3; the web dashboard does not display the clock.

---

## Hardware Bill of Materials & Wiring

### Bill of Materials (BOM)
| Component | Quantity | Notes |
| :--- | :--- | :--- |
| **ESP32 Development Board** | 1 | NodeMCU-32S / ESP32-WROOM-32 (2.4GHz Wi-Fi capable) |
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

### 1. Private Wi-Fi Credentials Setup (`.env`)
To prevent committing your Wi-Fi credentials to Git:
1. Copy `.env.example` to `.env`:
   ```bash
   cp .env.example .env
   ```
2. Open `.env` and enter your network credentials:
   ```env
   WIFI_SSID=YOUR_WIFI_NAME
   WIFI_PASSWORD=YOUR_WIFI_PASSWORD
   ```
> Note: PlatformIO automatically executes `read_env.py` during compilation to pass `.env` values into C++ preprocessor macros. `.env` is listed in `.gitignore` so your private credentials stay safe!

### 2. Build & Flash
Open terminal in the project directory:

```bash
# Build the project
platformio run

# Flash firmware to connected ESP32
platformio run --target upload

# Upload the web page to LittleFS
platformio run --target uploadfs

# Open Serial Monitor
platformio device monitor
```

The web interface is stored in `data/index.html` and served from LittleFS. Upload
the filesystem image separately from the firmware, and repeat the filesystem
upload whenever you change the page. Close the Serial Monitor before uploading
to a port it currently holds open.

---
