// ========== BLYNK CONFIGURATION ========== power suply + ke relay COM
// POWER SUPLY- ke -kipas
// kipas + ke NO 
#define BLYNK_TEMPLATE_ID "TMPL6qOTID8eO"
#define BLYNK_TEMPLATE_NAME "kipasanginnsmartttsiuuuuuuuu"
#define BLYNK_AUTH_TOKEN "L1QchigBqozJs5VTnPSQpUKpV9Us4mmd"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

// WiFi Credentials
const char* ssid = "Cristiano Ronaldo";
const char* password = "Ronaldo12";

// Database Server URL
const char* serverName = "http://192.168.18.60/Modul6/input.php";

// Pin Definitions
#define DHTPIN 4
#define DHTTYPE DHT11
#define LED_HIJAU 25
#define LED_KUNING 26
#define LED_MERAH 27
#define RELAY_KIPAS 32

DHT dht(DHTPIN, DHTTYPE);

// Variables
float temperature = 0;
float humidity = 0;
String fanStatus = "OFF";
String fanSpeed = "OFF";
String ledStatus = "HIJAU";
bool manualMode = false;
bool manualFanState = false;
String manualLedColor = "HIJAU";

// Timing Variables
unsigned long lastSendTime = 0;
unsigned long lastControlCheck = 0;
unsigned long lastSensorRead = 0;
unsigned long lastPrintTime = 0;
unsigned long manualOverrideTime = 0;

const long sendInterval = 5000;
const long controlInterval = 10000;
const long sensorInterval = 2000;
const long printInterval = 3000;
const long overrideDuration = 60000;   // 60 detik kembali AUTO

BlynkTimer timer;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Smart Fan System ===");
  
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(RELAY_KIPAS, OUTPUT);
  
  digitalWrite(LED_HIJAU, HIGH);
  digitalWrite(LED_KUNING, LOW);
  digitalWrite(LED_MERAH, LOW);
  digitalWrite(RELAY_KIPAS, HIGH);
  
  dht.begin();
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(1000L, sendToBlynk);
  
  Serial.println("System Ready!");
}

void loop() {
  Blynk.run();
  timer.run();
  
  unsigned long currentMillis = millis();
  
  // Auto-reset ke mode AUTO setelah timeout
  if (manualMode && (currentMillis - manualOverrideTime >= overrideDuration)) {
    manualMode = false;
    Serial.println("⏰ AUTO (Timeout - Kembali otomatis berdasarkan suhu)");
    Blynk.virtualWrite(V5, "AUTO");
    Blynk.virtualWrite(V6, 0);
    updateWebControl();
  }
  
  if (currentMillis - lastSensorRead >= sensorInterval) {
    readSensor();
    lastSensorRead = currentMillis;
  }
  
  if (currentMillis - lastControlCheck >= controlInterval) {
    getManualControl();
    lastControlCheck = currentMillis;
  }
  
  // Kontrol sistem
  if (manualMode) {
    applyManualControl();
  } else {
    autoControl();
  }
  
  if (currentMillis - lastSendTime >= sendInterval) {
    sendDataToServer();
    lastSendTime = currentMillis;
  }
  
  if (currentMillis - lastPrintTime >= printInterval) {
    printStatus();
    lastPrintTime = currentMillis;
  }
  
  delay(10);
}

void readSensor() {
  float tempRead = dht.readTemperature();
  float humRead = dht.readHumidity();
  
  if (!isnan(tempRead) && !isnan(humRead)) {
    temperature = tempRead;
    humidity = humRead;
  } else {
    Serial.println("DHT Error - using last valid reading");
  }
}

void autoControl() {
  if (temperature < 30) {
    digitalWrite(RELAY_KIPAS, HIGH);
    fanStatus = "OFF";
    fanSpeed = "OFF";
    
    digitalWrite(LED_HIJAU, HIGH);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH, LOW);
    ledStatus = "HIJAU";
    
  } else if (temperature >= 30 && temperature < 35) {
    digitalWrite(RELAY_KIPAS, LOW);
    fanStatus = "ON";
    fanSpeed = "SEDANG";
    
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_KUNING, HIGH);
    digitalWrite(LED_MERAH, LOW);
    ledStatus = "KUNING";
    
  } else {
    digitalWrite(RELAY_KIPAS, LOW);
    fanStatus = "ON";
    fanSpeed = "TINGGI";
    
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_KUNING, LOW);
    digitalWrite(LED_MERAH, HIGH);
    ledStatus = "MERAH";
  }
}

void applyManualControl() {
  digitalWrite(RELAY_KIPAS, manualFanState ? LOW : HIGH);
  fanStatus = manualFanState ? "ON" : "OFF";
  fanSpeed = manualFanState ? "MANUAL" : "OFF";
  
  digitalWrite(LED_HIJAU, manualLedColor == "HIJAU" ? HIGH : LOW);
  digitalWrite(LED_KUNING, manualLedColor == "KUNING" ? HIGH : LOW);
  digitalWrite(LED_MERAH, manualLedColor == "MERAH" ? HIGH : LOW);
  ledStatus = manualLedColor;
}

void getManualControl() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;
  String url = String(serverName) + "?action=getControl";
  
  http.begin(url);
  http.setTimeout(2000);
  
  int code = http.GET();
  
  if (code == 200) {
    String payload = http.getString();
    StaticJsonDocument<200> doc;
    
    if (deserializeJson(doc, payload) == DeserializationError::Ok) {
      manualMode = doc["manual_mode"];
      manualFanState = doc["fan_state"];
      manualLedColor = doc["led_color"].as<String>();
    }
  }
  http.end();
}

void sendDataToServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[DATABASE] ❌ WiFi tidak terhubung!");
    return;
  }
  
  HTTPClient http;
  
  String url = String(serverName) + "?action=sendData";
  url += "&temperature=" + String(temperature, 1);
  url += "&humidity=" + String(humidity, 1);
  url += "&fan_status=" + fanStatus;
  url += "&fan_speed=" + fanSpeed;
  url += "&led_status=" + ledStatus;
  url += "&mode=" + String(manualMode ? "MANUAL" : "AUTO");
  
  http.begin(url);
  http.setTimeout(3000);
  
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    if (httpCode == 200) {
      Serial.println("[DATABASE] ✓ Data berhasil terkirim ke database");
      Serial.printf("           → Temp: %.1f°C, Hum: %.1f%%, Fan: %s, LED: %s\n", 
                    temperature, humidity, fanStatus.c_str(), ledStatus.c_str());
    } else {
      Serial.printf("[DATABASE] ⚠ Server response code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[DATABASE] ❌ Gagal mengirim data: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

void printStatus() {
  String fanText = fanStatus;
  
  // Tambahkan detail kecepatan
  if (fanStatus == "ON") {
    fanText = "ON (" + fanSpeed + ")";
  }
  
  Serial.printf("Temp: %.1f°C | Hum: %.1f%% | Fan: %s | LED: %s | Mode: %s\n", 
                temperature, humidity, fanText.c_str(), 
                ledStatus.c_str(), manualMode ? "MANUAL" : "AUTO");
}

// ========== BLYNK FUNCTIONS ==========

void sendToBlynk() {
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, fanStatus);
  Blynk.virtualWrite(V3, ledStatus);
  Blynk.virtualWrite(V4, fanSpeed);
  Blynk.virtualWrite(V5, manualMode ? "MANUAL" : "AUTO");
}

// Tombol Mode (V6)
BLYNK_WRITE(V6) {
  int value = param.asInt();
  manualMode = (value == 1);
  
  if (manualMode) {
    manualOverrideTime = millis();
    Serial.println("🔧 Mode: MANUAL");
  } else {
    Serial.println("🤖 Mode: AUTO");
  }
  
  if (!manualMode) {
    autoControl();
  }
  
  updateWebControl();
}

// Tombol Kipas (V7) - Bisa diklik kapan saja
BLYNK_WRITE(V7) {
  int value = param.asInt();
  manualFanState = (value == 1);
  manualMode = true;
  manualOverrideTime = millis();
  
  Serial.println(manualFanState ? "🌀 Fan: ON (Override)" : "🌀 Fan: OFF (Override)");
  
  applyManualControl();
  updateWebControl();
  
  Blynk.virtualWrite(V5, "MANUAL");
  Blynk.virtualWrite(V6, 1);
}

// Menu LED (V8) - Bisa diklik kapan saja
BLYNK_WRITE(V8) {
  int value = param.asInt();
  manualMode = true;
  manualOverrideTime = millis();
  
  if (value == 1) manualLedColor = "HIJAU";
  else if (value == 2) manualLedColor = "KUNING";
  else if (value == 3) manualLedColor = "MERAH";
  
  Serial.println("💡 LED: " + manualLedColor + " (Override)");
  
  applyManualControl();
  updateWebControl();
  
  Blynk.virtualWrite(V5, "MANUAL");
  Blynk.virtualWrite(V6, 1);
}

void updateWebControl() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;
  
  String url = String(serverName) + "?action=updateControl";
  url += "&manual_mode=" + String(manualMode ? "1" : "0");
  url += "&fan_state=" + String(manualFanState ? "1" : "0");
  url += "&led_color=" + manualLedColor;
  
  http.begin(url);
  http.setTimeout(2000);
  http.GET();
  http.end();
}