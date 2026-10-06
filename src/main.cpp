#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <time.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WebServer server(80);

#define DHTPIN 13
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define BUTTON_PIN                                                             \
  25 // Pin connected to push button (other side connected to GND)

// --- Wi-Fi & NTP Time Configuration (Injected from .env via read_env.py) ---
#ifndef ENV_WIFI_SSID
#define ENV_WIFI_SSID "YOUR_WIFI_NAME"
#endif

#ifndef ENV_WIFI_PASSWORD
#define ENV_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

const char *WIFI_SSID = ENV_WIFI_SSID;
const char *WIFI_PASSWORD = ENV_WIFI_PASSWORD;
const char *NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 7200;     // Israel Timezone offset (UTC+2)
const int DAYLIGHT_OFFSET_SEC = 3600; // Daylight savings offset

// Wi-Fi state
bool wifiConnected = false;
bool littleFsReady = false;

// Page state and navigation (0 = Current, 1 = Min/Max, 2 = Date & Time)
int page = 0;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Telemetry state
unsigned long lastReadTime = 0;
unsigned long lastClockUpdate = 0;
float t = 0.0f;
float h = 0.0f;
float maxT = -100.0f;
float minT = 100.0f;
bool firstValidReading = true;
bool sensorError = false;

// 12x12 Sun / Hot Icon
static const unsigned char PROGMEM hot_sun_bmp[] = {
    0x09, 0x00, 0x22, 0x40, 0x07, 0x00, 0x0f, 0x80, 0x9f, 0x90, 0x9f, 0x90,
    0x0f, 0x80, 0x07, 0x00, 0x22, 0x40, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00};

// 12x12 Snowflake / Cold Icon
static const unsigned char PROGMEM cold_snowflake_bmp[] = {
    0x09, 0x00, 0x12, 0x40, 0x24, 0x80, 0x09, 0x00, 0x7f, 0xe0, 0x09, 0x00,
    0x24, 0x80, 0x12, 0x40, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Function to scan I2C bus on specified pins and return detected address (0 if
// none)
uint8_t scanI2C(int sda, int scl) {
  Serial.print(F("Scanning I2C bus (SDA="));
  Serial.print(sda);
  Serial.print(F(", SCL="));
  Serial.print(scl);
  Serial.println(F(")..."));

  Wire.begin(sda, scl);
  uint8_t foundAddr = 0;
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print(F(" -> FOUND I2C device at address 0x"));
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
      foundAddr = address;
    }
  }
  if (foundAddr == 0) {
    Serial.println(F(" -> No I2C devices found on these pins."));
  }
  return foundAddr;
}

void handleRoot() {
  if (!littleFsReady) {
    server.send(500, "text/plain", "LittleFS is unavailable");
    return;
  }

  File htmlFile = LittleFS.open("/index.html", "r");
  if (!htmlFile) {
    server.send(404, "text/plain", "Web page not found in LittleFS");
    return;
  }

  String html = htmlFile.readString();
  htmlFile.close();

  html.replace("{{ERROR_DISPLAY}}", sensorError ? "flex" : "none");
  html.replace("{{DATA_DISPLAY}}", sensorError ? "none" : "block");
  html.replace("{{TEMPERATURE}}", String(t, 1));
  html.replace("{{HUMIDITY}}", String(h, 1));
  html.replace("{{MIN_TEMPERATURE}}", String(minT, 1));
  html.replace("{{MAX_TEMPERATURE}}", String(maxT, 1));

  server.send(200, "text/html", html);
}

void setupWiFi() {
  Serial.print(F("Connecting to Wi-Fi: "));
  Serial.println(WIFI_SSID);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("ESP32 Weather Station"));
  display.println(F("---------------------"));
  display.println(F("Connecting Wi-Fi..."));
  display.println(WIFI_SSID);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();
  const unsigned long wifiTimeout = 10000; // 10 second connection timeout

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startAttemptTime < wifiTimeout) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;

    // Display IP on OLED screen
    display.println(F("Wi-Fi Connected!"));
    display.print(F("IP: "));
    display.println(WiFi.localIP());
    display.display();
    delay(2000);

    // Synchronize time via NTP
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    Serial.println(F("NTP Time synchronization initialized."));

    // Configure WebServer HTTP endpoints & start server
    server.on("/", handleRoot);
    server.begin();

    // Print prominent banner to Serial Terminal
    Serial.println(F("\n=================================================="));
    Serial.println(F("          WI-FI CONNECTED SUCCESSFULLY!           "));
    Serial.println(F("--------------------------------------------------"));
    Serial.print(F("  Browse to: http://"));
    Serial.println(WiFi.localIP());
    Serial.println(F("==================================================\n"));
  } else {
    wifiConnected = false;
    Serial.println(F("Wi-Fi Connection Timeout. Running in Offline Mode."));
    display.println(F("Wi-Fi Timeout!"));
    display.println(F("Running Offline Mode."));
    display.display();
    delay(1200);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("\n--- ESP32 Weather Station Starting ---"));

  // Configure push button pin with internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();

  // Try custom pins (27, 33) first, then fallback to standard ESP32 I2C pins
  // (21, 22)
  int sdaPin = 27;
  int sclPin = 33;
  uint8_t oledAddr = scanI2C(sdaPin, sclPin);

  if (oledAddr == 0) {
    sdaPin = 21;
    sclPin = 22;
    oledAddr = scanI2C(sdaPin, sclPin);
  }

  if (oledAddr == 0) {
    oledAddr = 0x3C;
    sdaPin = 27;
    sclPin = 33;
    Wire.begin(sdaPin, sclPin);
    Serial.println(F("WARNING: No I2C device detected by scan. Trying 0x3C on "
                     "pins (27,33)..."));
  }

  Wire.begin(sdaPin, sclPin);
  if (!display.begin(SSD1306_SWITCHCAPVCC, oledAddr, true, false)) {
    Serial.println(F("ERROR: SSD1306 allocation failed!"));
    for (;;)
      ;
  }

  Serial.println(F("OLED Display Initialized successfully!"));

  littleFsReady = LittleFS.begin();
  if (littleFsReady) {
    Serial.println(F("LittleFS mounted successfully."));
  } else {
    Serial.println(
        F("ERROR: LittleFS mount failed. Upload the filesystem image."));
  }

  // Attempt Wi-Fi Connection and NTP Sync
  setupWiFi();
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);

  if (sensorError) {
    display.println(F("ESP32 Weather Station"));
    display.println(F("---------------------"));
    display.println(F("Sensor Error!"));
    display.println(F("Check DHT11 wiring."));
    display.display();
    return;
  }

  if (page == 0) {
    // Page 0: Current Telemetry & Icons
    display.println(F("ESP32 Weather Station"));
    display.println(F("---------------------"));

    display.print(F("Temp: "));
    display.print(t);
    display.print(F(" "));
    display.write(247);
    display.println(F("C"));

    // Draw Hot/Cold status with icon
    display.setCursor(0, 26);
    if (t >= 25.0f) {
      display.drawBitmap(0, 25, hot_sun_bmp, 12, 12, SSD1306_WHITE);
      display.setCursor(16, 26);
      display.println(F("It's hot in here!"));
    } else {
      display.drawBitmap(0, 25, cold_snowflake_bmp, 12, 12, SSD1306_WHITE);
      display.setCursor(16, 26);
      display.println(F("It's cold in here!"));
    }

    display.setCursor(0, 38);
    display.print(F("Humidity: "));
    display.print(h);
    display.println(F(" %"));

    display.setCursor(0, 54);
    display.println(F("[Page 1/3: Base]"));
  } else if (page == 1) {
    // Page 1: Min / Max Temperature Statistics
    display.println(F("-- MIN / MAX STATS --"));
    display.println(F("---------------------"));

    display.print(F("Max Temp: "));
    display.print(maxT);
    display.print(F(" "));
    display.write(247);
    display.println(F("C"));

    display.print(F("Min Temp: "));
    display.print(minT);
    display.print(F(" "));
    display.write(247);
    display.println(F("C"));

    display.print(F("Cur Temp: "));
    display.print(t);
    display.print(F(" "));
    display.write(247);
    display.println(F("C"));

    display.setCursor(0, 38);
    display.print(F("Humidity: "));
    display.print(h);
    display.println(F(" %"));

    display.setCursor(0, 54);
    display.println(F("[Page 2/3: Min/Max]"));
  } else if (page == 2) {
    // Page 2: Live Clock & Date
    display.println(F("--- DATE & TIME ---"));
    display.println(F("-------------------"));

    struct tm timeinfo;
    if (wifiConnected && getLocalTime(&timeinfo)) {
      char dateStr[20];
      char timeStr[20];
      strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
      strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

      display.print(F("Date: "));
      display.println(dateStr);

      display.print(F("Time: "));
      display.println(timeStr);

      display.setCursor(0, 32);
      display.print(F("WiFi: Connected"));
    } else {
      display.println(F("Date: N/A"));
      display.println(F("Time: N/A"));
      display.setCursor(0, 32);
      display.println(F("WiFi: Offline Mode"));
    }

    if (wifiConnected) {
      display.setCursor(0, 40);
      display.println(F("Website: http://"));
      display.setCursor(0, 48);
      display.println(WiFi.localIP());
    } else {
      display.setCursor(0, 40);
      display.println(F("Website: unavailable"));
    }

    display.setCursor(0, 56);
    display.println(F("[Page 3/3: Clock]"));
  }

  display.display();
}

void loop() {
  // 0. Process Web Server Requests
  if (wifiConnected) {
    server.handleClient();
  }

  // 1. Non-blocking Button Read & Debounce
  bool currentReading = digitalRead(BUTTON_PIN);
  if (currentReading != lastButtonState) {
    lastDebounceTime = millis();
    lastButtonState = currentReading;
  }

  static bool buttonPressedState = HIGH;
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentReading != buttonPressedState) {
      buttonPressedState = currentReading;
      // On button press (LOW transition when using INPUT_PULLUP)
      if (buttonPressedState == LOW) {
        page = (page + 1) % 3; // Cycles 0 -> 1 -> 2 -> 0
        Serial.print(F("Button pressed. Switched to Page "));
        Serial.println(page);
        updateDisplay();
      }
    }
  }

  // 2. Non-blocking Sensor Read Every 2 Seconds
  if (millis() - lastReadTime >= 2000 || lastReadTime == 0) {
    lastReadTime = millis();

    float readH = dht.readHumidity();
    float readT = dht.readTemperature();

    if (isnan(readH) || isnan(readT)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      sensorError = true;
    } else {
      sensorError = false;
      h = readH;
      t = readT;

      if (firstValidReading) {
        maxT = t;
        minT = t;
        firstValidReading = false;
      } else {
        if (t > maxT)
          maxT = t;
        if (t < minT)
          minT = t;
      }
    }

    if (page != 2) {
      updateDisplay();
    }
  }

  // 3. Live Clock Update Every 1 Second when on Page 2
  if (page == 2 && (millis() - lastClockUpdate >= 1000)) {
    lastClockUpdate = millis();
    updateDisplay();
  }
}