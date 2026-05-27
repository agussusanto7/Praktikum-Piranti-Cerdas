#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Agus Depan";
const char* password = "satusampai9";

// Server URL - GANTI DENGAN IP LAPTOP KAMU
const char* serverUrl = "http://192.168.101.3/coba/api/"; 

// Pin setup
#define LED_PIN 2
#define SERVO_PIN 5
#define LDR_PIN 34

Servo myservo;

// Variables
bool autoMode = true;
bool ledState = false;
int servoAngle = 0;
unsigned long lastControlUpdate = 0;

// Fungsi untuk membaca sensor LDR (0-100)
int readLDR() {
  int raw = analogRead(LDR_PIN);
  return map(raw, 0, 4095, 0, 100);
}

// Fungsi untuk kirim data ke server
void sendDataToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    int ldr = readLDR();
    
    // Buat URL dengan parameter
    String url = String(serverUrl) + "save-data.php?";
    url += "ldr=" + String(ldr);
    url += "&servo=" + String(servoAngle);
    url += "&led=" + String(ledState ? 1 : 0);
    url += "&mode=" + String(autoMode ? "auto" : "manual");
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Data sent successfully");
    } else {
      Serial.println("Error sending data: " + String(httpCode));
    }
    
    http.end();
  }
}

// Fungsi untuk ambil kontrol dari server (DIPERBAIKI dengan ArduinoJson)
void getControlFromServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = String(serverUrl) + "get-data.php";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.GET();
    
    if (httpCode == 200) {
      String payload = http.getString();
      
      // Parse JSON
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        // Ambil data controls
        JsonObject controls = doc["controls"];
        
        if (!controls.isNull()) {
          // Update mode
          const char* newMode = controls["mode"];
          if (newMode != nullptr) {
            bool newAutoMode = (String(newMode) == "auto");
            if (newAutoMode != autoMode) {
              autoMode = newAutoMode;
              Serial.println("Mode changed to: " + String(autoMode ? "AUTO" : "MANUAL"));
            }
          }
          
          // Update servo (hanya jika mode manual)
          if (!autoMode) {
            int targetAngle = controls["servo_target"];
            if (targetAngle != servoAngle) {
              servoAngle = constrain(targetAngle, 0, 180);
              myservo.write(servoAngle);
              Serial.println("Servo moved to: " + String(servoAngle));
            }
          }
          
          // Update LED
          int ledCommand = controls["led_command"];
          bool newLedState = (ledCommand == 1);
          if (newLedState != ledState) {
            ledState = newLedState;
            digitalWrite(LED_PIN, ledState ? HIGH : LOW);
            Serial.println("LED: " + String(ledState ? "ON" : "OFF"));
          }
        }
      } else {
        Serial.println("JSON parsing error: " + String(error.c_str()));
      }
    } else {
      Serial.println("HTTP error: " + String(httpCode));
    }
    
    http.end();
  }
}

void setup() {
  Serial.begin(115200);
  
  // Setup pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  
  // Setup servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 500, 2400);
  
  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("System ready!");
}

void loop() {
  static unsigned long lastSend = 0;
  static unsigned long lastGet = 0;
  
  // Kirim data setiap 2 detik
  if (millis() - lastSend > 2000) {
    sendDataToServer();
    lastSend = millis();
  }
  
  // Ambil kontrol dari server setiap 500ms (lebih responsif)
  if (millis() - lastGet > 500) {
    getControlFromServer();
    lastGet = millis();
  }
  
  // Mode AUTO: kontrol servo berdasarkan LDR
  if (autoMode) {
    int ldrValue = readLDR();
    
    // Jika cahaya terang (>60), buka gorden (180°)
    // Jika cahaya gelap (<40), tutup gorden (0°)
    if (ldrValue > 60 && servoAngle != 180) {
      servoAngle = 180;
      myservo.write(servoAngle);
      Serial.println("AUTO: Gorden dibuka (cahaya terang)");
    } else if (ldrValue < 40 && servoAngle != 0) {
      servoAngle = 0;
      myservo.write(servoAngle);
      Serial.println("AUTO: Gorden ditutup (cahaya gelap)");
    }
  }
  
  delay(100);
}