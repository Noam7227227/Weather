#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 13 // Pin connected to DHT11 data pin
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define BUTTON_PIN                                                             \
  25 // Pin connected to push button (other side connected to GND)

// Page state and navigation
int page = 0; // 0 = Current Weather, 1 = Min / Max Statistics
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // Ms

// Telemetry state
unsigned long lastReadTime = 0;
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

  // Startup Screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("ESP32 Weather Station"));
  display.println(F("---------------------"));
  display.println(F("Initializing..."));
  display.println(F("Button Pin: GPIO 25"));
  display.display();
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
    display.println(F("[Page 1/2: Current]"));
  } else {
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
    display.println(F("[Page 2/2: Min/Max]"));
  }

  display.display();
}

void loop() {
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
        page = (page == 0) ? 1 : 0;
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

      // Log to Serial
      Serial.print(F("Humidity: "));
      Serial.print(h);
      Serial.print(F("%  Temperature: "));
      Serial.print(t);
      Serial.print(F("°C  [Min: "));
      Serial.print(minT);
      Serial.print(F("°C, Max: "));
      Serial.print(maxT);
      Serial.println(F("°C]"));
    }

    updateDisplay();
  }
}