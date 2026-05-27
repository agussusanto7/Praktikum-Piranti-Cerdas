/*
 * ESP32 Distance Monitoring & Control System
 * Sistem kontrol LED & Buzzer dengan sensor HC-SR04
 * Version: 2.0 (Optimized & Clean Code)
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ==================== KONFIGURASI ====================
// WiFi Credentials
const char* WIFI_SSID = "Cristiano Ronaldo";
const char* WIFI_PASSWORD = "Ronaldo12";

// Pin Configuration
const uint8_t PIN_LED = 4;
const uint8_t PIN_BUZZER = 18;
const uint8_t PIN_TRIG = 26;
const uint8_t PIN_ECHO = 27;

// System Configuration
const float DISTANCE_THRESHOLD = 10.0;  // cm
const unsigned long MEASURE_INTERVAL = 500;  // ms
const unsigned long PULSE_TIMEOUT = 30000;  // microseconds
const uint8_t WIFI_TIMEOUT = 30;  // seconds

// ==================== GLOBAL VARIABLES ====================
WebServer server(80);

struct SystemState {
    float distance;
    bool ledOn;
    bool buzzerOn;
    bool autoMode;
    unsigned long lastMeasurement;
} state = {0.0, false, false, true, 0};

// ==================== DISTANCE MEASUREMENT ====================
float measureDistance() {
    // Trigger pulse
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    
    // Measure echo
    long duration = pulseIn(PIN_ECHO, HIGH, PULSE_TIMEOUT);
    float distance = (duration * 0.034) / 2.0;
    
    // Validate range
    if (distance > 0 && distance < 400) {
        Serial.printf("Jarak: %.1f cm\n", distance);
        return distance;
    }
    return -1.0;
}

// ==================== DEVICE CONTROL ====================
void setDevices(bool led, bool buzzer) {
    digitalWrite(PIN_LED, led ? HIGH : LOW);
    digitalWrite(PIN_BUZZER, buzzer ? HIGH : LOW);
    state.ledOn = led;
    state.buzzerOn = buzzer;
}

void handleAutoMode() {
    if (!state.autoMode || state.distance <= 0) return;
    
    bool shouldActivate = (state.distance <= DISTANCE_THRESHOLD);
    
    if (shouldActivate && (!state.ledOn || !state.buzzerOn)) {
        setDevices(true, true);
        Serial.println("⚠️  AUTO: Objek terdeteksi dekat - Alarm aktif");
    } 
    else if (!shouldActivate && state.distance < 400 && (state.ledOn || state.buzzerOn)) {
        setDevices(false, false);
        Serial.println("✅ AUTO: Jarak aman - Alarm nonaktif");
    }
}

// ==================== HTML GENERATOR ====================
String generateHTML() {
    String statusClass = (state.distance > 0 && state.distance <= DISTANCE_THRESHOLD) 
                        ? "status-alarm" : "status-normal";
    String statusIcon = (state.distance > 0 && state.distance <= DISTANCE_THRESHOLD)
                       ? "fa-exclamation-triangle spin" : "fa-check-circle";
    String statusText = (state.distance > 0 && state.distance <= DISTANCE_THRESHOLD)
                       ? "PERINGATAN! Objek Terlalu Dekat!" : "Jarak Aman";
    
    String html = F("<!DOCTYPE html><html><head>"
        "<meta http-equiv='refresh' content='2'/>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'>"
        "<title>ESP32 IoT Control</title><style>"
        "*{margin:0;padding:0;box-sizing:border-box}"
        "body{font-family:'Segoe UI',sans-serif;background:linear-gradient(135deg,#667eea,#764ba2);"
        "min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px}"
        ".container{background:rgba(255,255,255,0.98);border-radius:30px;padding:40px;"
        "box-shadow:0 30px 90px rgba(0,0,0,0.5);max-width:500px;width:100%}"
        ".header{text-align:center;margin-bottom:30px}"
        ".header h1{color:#1a202c;font-size:1.6rem;font-weight:800;margin-bottom:8px}"
        ".header .sub{color:#667eea;font-size:0.9rem;font-weight:600;margin-top:5px}"
        ".status-indicator{display:inline-block;width:10px;height:10px;background:#10b981;"
        "border-radius:50%;margin-right:8px;animation:pulse 2s infinite}"
        ".card{border-radius:25px;padding:30px;margin-bottom:20px;box-shadow:0 15px 45px rgba(0,0,0,0.3)}"
        ".card-icon{font-size:3rem;margin-bottom:15px}"
        ".card-label{color:rgba(255,255,255,0.95);font-size:1rem;font-weight:600;margin-bottom:10px}"
        ".card-value{color:#fff;font-size:3.5rem;font-weight:800;line-height:1}"
        ".card-unit{color:rgba(255,255,255,0.9);font-size:1.2rem;font-weight:600}"
        ".distance-card{background:linear-gradient(135deg,#667eea,#764ba2)}"
        ".status-card{background:linear-gradient(135deg,#f093fb,#f5576c)}"
        ".device-status{color:#fff;padding:8px 16px;border-radius:20px;font-size:0.8rem;"
        "font-weight:700;margin-top:10px;display:inline-block}"
        ".device-on{background:linear-gradient(135deg,#06b6d4,#0891b2);animation:pulse 1.5s infinite}"
        ".device-off{background:linear-gradient(135deg,#64748b,#475569)}"
        ".mode-status{background:linear-gradient(135deg,#f59e0b,#d97706);color:#fff;"
        "padding:6px 12px;border-radius:15px;font-size:0.75rem;font-weight:700;margin-top:8px;display:inline-block}"
        ".btn-group{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-top:20px}"
        ".btn{padding:12px 20px;font-size:0.9rem;font-weight:bold;border:none;border-radius:12px;"
        "cursor:pointer;transition:all 0.3s;text-decoration:none;text-align:center;color:#fff}"
        ".btn:hover{transform:translateY(-3px)}"
        ".btn-on{background:linear-gradient(135deg,#10b981,#059669)}"
        ".btn-off{background:linear-gradient(135deg,#ef4444,#dc2626)}"
        ".btn-all{grid-column:1/-1;padding:15px;font-size:1rem}"
        ".status-normal{background:linear-gradient(135deg,#10b981,#059669);color:#fff;"
        "padding:8px 16px;border-radius:20px;font-size:0.8rem;font-weight:700;margin-top:12px;display:inline-block}"
        ".status-alarm{background:linear-gradient(135deg,#ef4444,#dc2626);color:#fff;"
        "padding:8px 16px;border-radius:20px;font-size:0.8rem;font-weight:700;margin-top:12px;"
        "display:inline-block;animation:blink 1s infinite}"
        ".footer{text-align:center;margin-top:30px;color:#64748b;font-size:0.85rem}"
        ".spin{display:inline-block;animation:rotate 2s linear infinite}"
        "@keyframes blink{0%,100%{opacity:1}50%{opacity:0.3}}"
        "@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.5}}"
        "@keyframes rotate{from{transform:rotate(0deg)}to{transform:rotate(360deg)}}"
        "</style></head><body><div class='container'>"
        "<div class='header'><h1><i class='fas fa-microchip'></i> Sistem Monitoring Jarak HC-SR04</h1>"
        "<p class='sub'>ESP32 Smart IoT System</p>"
        "<p><span class='status-indicator'></span>Real-time Monitoring</p></div>");
    
    // Distance Card
    html += F("<div class='card distance-card'><div class='card-icon'><i class='fas fa-ruler'></i></div>"
              "<div class='card-label'>Jarak Terukur</div><div class='card-value'>");
    html += (state.distance > 0) ? String(state.distance, 1) : "--";
    html += F("<span class='card-unit'> cm</span></div></div>");
    
    // Status Card
    html += F("<div class='card status-card'><div class='card-icon'><i class='fas fa-signal'></i></div>"
              "<div class='card-label'>Status Perangkat</div><div style='text-align:left;margin-top:15px'>"
              "<p style='color:#fff;font-size:0.9rem;margin-bottom:8px'><i class='fas fa-lightbulb'></i> LED: "
              "<span class='device-status ");
    html += state.ledOn ? "device-on'>MENYALA" : "device-off'>MATI";
    html += F("</span></p><p style='color:#fff;font-size:0.9rem;margin-bottom:8px'>"
              "<i class='fas fa-volume-up'></i> Buzzer: <span class='device-status ");
    html += state.buzzerOn ? "device-on'>MENYALA" : "device-off'>MATI";
    html += F("</span></p><p style='color:#fff;font-size:0.9rem'><i class='fas fa-cog'></i> Mode: "
              "<span class='mode-status'>");
    html += state.autoMode ? "OTOMATIS" : "MANUAL";
    html += F("</span></p></div></div>");
    
    // Control Panel
    html += F("<div class='card' style='background:linear-gradient(135deg,#f8f9fa,#e9ecef);padding:25px'>"
              "<h3 style='color:#1a202c;margin-bottom:15px'><i class='fas fa-gamepad'></i> Panel Kontrol</h3>"
              "<div class='btn-group'>"
              "<a href='/led/on' class='btn btn-on'><i class='fas fa-lightbulb'></i> LED ON</a>"
              "<a href='/led/off' class='btn btn-off'><i class='fas fa-lightbulb'></i> LED OFF</a>"
              "<a href='/buzzer/on' class='btn btn-on'><i class='fas fa-volume-up'></i> BUZZER ON</a>"
              "<a href='/buzzer/off' class='btn btn-off'><i class='fas fa-volume-up'></i> BUZZER OFF</a>"
              "<a href='/all/on' class='btn btn-all btn-on'><i class='fas fa-power-off'></i> SEMUA ON</a>"
              "<a href='/all/off' class='btn btn-all btn-off'><i class='fas fa-power-off'></i> SEMUA OFF</a>"
              "</div></div>");
    
    // Footer
    html += F("<div class='footer'><p style='font-weight:700;color:#1e293b;margin-bottom:10px'>"
              "<i class='fas fa-wifi'></i> IP: ");
    html += WiFi.localIP().toString();
    html += F("</p><div class='");
    html += statusClass;
    html += F("'><i class='fas ");
    html += statusIcon;
    html += F("'></i> ");
    html += statusText;
    html += F("</div></div></div></body></html>");
    
    return html;
}

// ==================== WEB HANDLERS ====================
void handleRoot() {
    state.distance = measureDistance();
    handleAutoMode();
    server.send(200, "text/html", generateHTML());
}

void handleLED(bool turnOn) {
    setDevices(turnOn, state.buzzerOn);
    state.autoMode = false;
    Serial.printf("📱 Web: LED %s\n", turnOn ? "ON" : "OFF");
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleBuzzer(bool turnOn) {
    setDevices(state.ledOn, turnOn);
    state.autoMode = false;
    Serial.printf("📱 Web: Buzzer %s\n", turnOn ? "ON" : "OFF");
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAll(bool turnOn) {
    setDevices(turnOn, turnOn);
    state.autoMode = !turnOn;  // Auto mode ON saat semua OFF
    Serial.printf("📱 Web: Semua %s (Auto: %s)\n", 
                  turnOn ? "ON" : "OFF", 
                  state.autoMode ? "ON" : "OFF");
    server.sendHeader("Location", "/");
    server.send(303);
}

// ==================== SYSTEM INITIALIZATION ====================
void initPins() {
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    setDevices(false, false);
}

void playBootTone() {
    for (uint8_t i = 0; i < 2; i++) {
        digitalWrite(PIN_BUZZER, HIGH);
        delay(200);
        digitalWrite(PIN_BUZZER, LOW);
        delay(200);
    }
}

void playSuccessTone() {
    for (uint8_t i = 0; i < 3; i++) {
        digitalWrite(PIN_BUZZER, HIGH);
        delay(100);
        digitalWrite(PIN_BUZZER, LOW);
        delay(100);
    }
}

void playErrorTone() {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(100);
    digitalWrite(PIN_BUZZER, LOW);
    delay(900);
}

bool connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    
    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_TIMEOUT) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ WiFi Connected!");
        Serial.printf("SSID: %s\n", WIFI_SSID);
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Signal: %d dBm\n", WiFi.RSSI());
        return true;
    }
    
    Serial.println("❌ WiFi Connection Failed!");
    return false;
}

void setupWebServer() {
    server.on("/", handleRoot);
    server.on("/led/on", []() { handleLED(true); });
    server.on("/led/off", []() { handleLED(false); });
    server.on("/buzzer/on", []() { handleBuzzer(true); });
    server.on("/buzzer/off", []() { handleBuzzer(false); });
    server.on("/all/on", []() { handleAll(true); });
    server.on("/all/off", []() { handleAll(false); });
    server.begin();
}

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("ESP32 Distance Monitoring System v2.0");
    Serial.println("========================================");
    
    initPins();
    playBootTone();
    
    if (!connectWiFi()) {
        while (true) { playErrorTone(); }
    }
    
    playSuccessTone();
    
    if (MDNS.begin("esp32")) {
        Serial.println("mDNS: http://esp32.local");
    }
    
    setupWebServer();
    Serial.printf("Threshold: %.1f cm\n", DISTANCE_THRESHOLD);
    Serial.println("========================================\n");
}

// ==================== MAIN LOOP ====================
void loop() {
    server.handleClient();
    
    unsigned long currentTime = millis();
    if (currentTime - state.lastMeasurement >= MEASURE_INTERVAL) {
        state.lastMeasurement = currentTime;
        state.distance = measureDistance();
        handleAutoMode();
    }
    
    delay(2);
}