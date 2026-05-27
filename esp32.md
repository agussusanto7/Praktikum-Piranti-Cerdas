# Smart Greenhouse IoT - ESP32 Code (Updated)

## Kode Arduino IDE untuk ESP32

Copy dan paste kode ini ke Arduino IDE untuk di-upload ke ESP32 Anda.

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== KONFIGURASI WIFI =====
const char* ssid = "Bara";
const char* password = "akbarganteng";

// ===== KONFIGURASI SERVER =====
// GANTI dengan IP komputer kamu (cek pakai ipconfig)
const char* serverURL = "http://192.168.238.163/UAPIOT5/api/save-data.php";

// ===== KONFIGURASI PIN =====
const int DHT_PIN = 26;
const int BUZZER_PIN = 25;
const int SERVO_PIN = 27;
const int FAN_PIN = 32;
const int LCD_SDA = 21;
const int LCD_SCL = 22;

// ===== THRESHOLD =====
float TEMP_THRESHOLD = 30.0;

// ===== OBJEK =====
DHT dht(DHT_PIN, DHT11);
Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== VARIABEL =====
bool windowOpen = false;
bool fanActive = false;
bool alarmTriggered = false;
String currentMode = "auto";
float currentTemp = 0;
float currentHum = 0;
int webWindowCmd = 0;
int webFanCmd = 0;

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 3000;

unsigned long lastBuzzerTime = 0;
bool buzzerState = false;

const int SERVO_OPEN = 90;   // Sudut 90 derajat = Jendela TERBUKA
const int SERVO_CLOSE = 0;   // Sudut 0 derajat = Jendela TERTUTUP

void setup() {
    Serial.begin(115200);

    Wire.begin(LCD_SDA, LCD_SCL);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SMART GREENHOUSE");
    lcd.setCursor(0, 1);
    lcd.print("HTTP Version");
    delay(2000);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(FAN_PIN, HIGH);

    dht.begin();

    myServo.setPeriodHertz(50);
    myServo.attach(SERVO_PIN);
    myServo.write(SERVO_CLOSE);
    delay(500);
    myServo.detach();

    Serial.println("\n=== SMART GREENHOUSE HTTP ===");

    connectWiFi();

    for(int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
    }

    lcd.clear();
    lcd.print("System Ready!");
    delay(1500);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }

    readSensors();

    if (millis() - lastSendTime >= SEND_INTERVAL) {
        sendDataToServer();
        lastSendTime = millis();
    }

    if (currentMode == "auto") {
        autoControl();
    } else {
        manualControl();
    }

    handleBuzzerAlarm();
    updateLCD();

    delay(100);
}

void connectWiFi() {
    Serial.print("Connecting to WiFi");
    lcd.clear();
    lcd.print("Connecting WiFi");

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.println(WiFi.localIP());
        lcd.clear();
        lcd.print("WiFi OK!");
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        delay(2000);
    } else {
        Serial.println("\nWiFi Failed!");
        lcd.clear();
        lcd.print("WiFi FAILED!");
        delay(2000);
    }
}

void readSensors() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t)) currentTemp = t;
    if (!isnan(h)) currentHum = h;

    Serial.print("Temp: ");
    Serial.print(currentTemp);
    Serial.print("C | Hum: ");
    Serial.print(currentHum);
    Serial.println("%");
}

void sendDataToServer() {
    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;

    String url = String(serverURL);
    url += "?temp=" + String(currentTemp, 1);
    url += "&hum=" + String(currentHum, 1);

    // LOGIKA STATUS JENDELA (SUDAH DIPERBAIKI):
    // windowOpen = true (TERBUKA) -> kirim 0
    // windowOpen = false (TERTUTUP) -> kirim 1
    url += "&window=" + String(windowOpen ? 0 : 1);

    url += "&fan=" + String(fanActive ? 1 : 0);
    url += "&alarm=" + String(alarmTriggered ? 1 : 0);
    url += "&mode=" + currentMode;

    Serial.println("Sending: " + url);

    http.begin(url);
    http.setTimeout(5000);

    int httpCode = http.GET();

    if (httpCode > 0) {
        String response = http.getString();
        Serial.println("Response: " + response);

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, response);

        if (!error && doc["status"] == "success") {
            JsonObject controls = doc["controls"];

            String newMode = controls["mode"].as<String>();
            if (newMode != currentMode) {
                currentMode = newMode;
                Serial.println("Mode: " + currentMode);
            }

            webWindowCmd = controls["window_cmd"].as<int>();
            webFanCmd = controls["fan_cmd"].as<int>();

            float newThreshold = controls["threshold"].as<float>();
            if (newThreshold != TEMP_THRESHOLD) {
                TEMP_THRESHOLD = newThreshold;
                Serial.print("Threshold: ");
                Serial.println(TEMP_THRESHOLD);
            }
        }
    } else {
        Serial.print("HTTP Error: ");
        Serial.println(httpCode);
    }

    http.end();
}

void autoControl() {
    if (currentTemp > TEMP_THRESHOLD) {
        if (!alarmTriggered) {
            alarmTriggered = true;
            Serial.println("ALARM: Suhu tinggi!");
        }
        if (!windowOpen) openWindow();
        if (!fanActive) turnFanOn();
    } else {
        if (alarmTriggered) {
            alarmTriggered = false;
            digitalWrite(BUZZER_PIN, HIGH);
            delay(100);
            digitalWrite(BUZZER_PIN, LOW);
        }
        if (windowOpen) closeWindow();
        if (fanActive) turnFanOff();
    }
}

void manualControl() {
    if (webWindowCmd == 0 && !windowOpen) openWindow();    // cmd 0 = BUKA
    else if (webWindowCmd == 1 && windowOpen) closeWindow(); // cmd 1 = TUTUP

    if (webFanCmd == 1 && !fanActive) turnFanOn();
    else if (webFanCmd == 0 && fanActive) turnFanOff();

    alarmTriggered = (currentTemp > TEMP_THRESHOLD);
}

void openWindow() {
    Serial.println("Opening window...");
    myServo.attach(SERVO_PIN);
    delay(15);
    myServo.write(SERVO_OPEN);   // 90 derajat = TERBUKA
    delay(500);
    myServo.detach();
    windowOpen = true;
}

void closeWindow() {
    Serial.println("Closing window...");
    myServo.attach(SERVO_PIN);
    delay(15);
    myServo.write(SERVO_CLOSE);  // 0 derajat = TERTUTUP
    delay(500);
    myServo.detach();
    windowOpen = false;
}

void turnFanOn() {
    digitalWrite(FAN_PIN, LOW);
    fanActive = true;
    Serial.println("Fan ON");
}

void turnFanOff() {
    digitalWrite(FAN_PIN, HIGH);
    fanActive = false;
    Serial.println("Fan OFF");
}

void handleBuzzerAlarm() {
    if (alarmTriggered) {
        if (millis() - lastBuzzerTime >= 500) {
            buzzerState = !buzzerState;
            digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
            lastBuzzerTime = millis();
        }
    } else {
        digitalWrite(BUZZER_PIN, LOW);
        buzzerState = false;
    }
}

void updateLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(currentTemp, 1);
    lcd.print("C H:");
    lcd.print(currentHum, 0);
    lcd.print("%");

    lcd.setCursor(0, 1);
    if (alarmTriggered) {
        lcd.print("ALARM! ");
    } else {
        lcd.print(currentMode == "auto" ? "AUTO " : "MAN  ");
    }
    lcd.print("W:");
    lcd.print(windowOpen ? "OP" : "CL");
    lcd.print(" F:");
    lcd.print(fanActive ? "ON" : "OF");
}
```

## Wiring Diagram

```
ESP32 Pinout:

DHT11:
- VCC  -> 3.3V
- DATA -> GPIO 26
- GND  -> GND

Servo SG90:
- VCC (Red)    -> 5V
- GND (Brown)  -> GND
- Signal (Orange) -> GPIO 27

DC Fan (via Relay/Transistor):
- VCC -> 5V
- GND -> GND
- Control -> GPIO 32

Buzzer:
- Positive (+) -> GPIO 25
- Negative (-) -> GND

LCD I2C:
- SDA -> GPIO 21
- SCL -> GPIO 22
- VCC -> 5V
- GND -> GND
```

## Logika Status Jendela (SUDAH DIPERBAIKI)

| Variabel ESP32 | Kirim ke Server | Tampilan di Web | Kondisi Fisik | Servo Position |
|----------------|-----------------|-----------------|---------------|----------------|
| `windowOpen = true` | 0 | 🟢 Terbuka | Jendela TERBUKA | 90° |
| `windowOpen = false` | 1 | 🔴 Tertutup | Jendela TERTUTUP | 0° |

## Perubahan yang Dilakukan

1. **Logika pengiriman data jendela dibalik:**
   - Sebelumnya: `windowOpen ? 1 : 0`
   - Sekarang: `windowOpen ? 0 : 1`

2. **Logika manual control disesuaikan:**
   - `webWindowCmd == 0` → `openWindow()` (BUKA)
   - `webWindowCmd == 1` → `closeWindow()` (TUTUP)

## Cara Upload ke ESP32

1. Install Arduino IDE: https://www.arduino.cc/en/software

2. Install ESP32 Board:
   - File → Preferences
   - Additional Board Manager URLs: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - OK
   - Tools → Board → Boards Manager
   - Cari "esp32" dan install

3. Install Libraries (Sketch → Include Library → Manage Libraries):
   - "DHT sensor library" by Adafruit
   - "ESP32Servo" by Kevin Harrington
   - "ArduinoJson" by Benoit Blanchon
   - "LiquidCrystal I2C" by Frank de Brabander

4. Ganti WiFi dan Server URL di kode atas

5. Select Board: Tools → Board → ESP32 Arduino → ESP32 Dev Module

6. Select Port dan Upload

---

**Dibuat untuk Smart Greenhouse IoT System**
