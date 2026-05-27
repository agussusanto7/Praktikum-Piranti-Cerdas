/*
 * ╔════════════════════════════════════════════════════════════╗
 * ║  ESP32 ULTRA RESPONSIVE IoT DASHBOARD                     ║
 * ║  Next-Gen Distance Monitoring & Control System            ║
 * ║  Version: 4.1 AJAX SMOOTH UPDATE EDITION                  ║
 * ╚════════════════════════════════════════════════════════════╝
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>

// ═══════════════════ KONFIGURASI ═══════════════════
const char* WIFI_SSID = "Cristiano Ronaldo";
const char* WIFI_PASSWORD = "Ronaldo12";

// Pin Configuration
const uint8_t PIN_LED = 4;
const uint8_t PIN_BUZZER = 18;
const uint8_t PIN_TRIG = 26;
const uint8_t PIN_ECHO = 27;

// System Configuration
const float DISTANCE_THRESHOLD = 10.0;
const unsigned long MEASURE_INTERVAL = 300;
const unsigned long PULSE_TIMEOUT = 30000;

// ═══════════════════ GLOBAL STATE ═══════════════════
WebServer server(80);

struct SystemState {
    float distance;
    float minDistance;
    float maxDistance;
    bool ledOn;
    bool buzzerOn;
    bool autoMode;
    unsigned long lastMeasurement;
    int measurementCount;
    float avgDistance;
    bool wasAlertActive;
} state = {0.0, 400.0, 0.0, false, false, true, 0, 0, 0.0, false};

// ═══════════════════ SENSOR FUNCTIONS ═══════════════════
float measureDistance() {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    
    long duration = pulseIn(PIN_ECHO, HIGH, PULSE_TIMEOUT);
    float distance = (duration * 0.034) / 2.0;
    
    if (distance > 0 && distance < 400) {
        state.measurementCount++;
        state.avgDistance = ((state.avgDistance * (state.measurementCount - 1)) + distance) / state.measurementCount;
        if (distance < state.minDistance) state.minDistance = distance;
        if (distance > state.maxDistance) state.maxDistance = distance;
        return distance;
    }
    return -1.0;
}

// ═══════════════════ DEVICE CONTROL ═══════════════════
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
        Serial.println("╔════════════════════════════════════════╗");
        Serial.println("║           ⚠️  PERINGATAN!              ║");
        Serial.println("╚════════════════════════════════════════╝");
        Serial.printf("🚨 Objek terlalu dekat! Jarak: %.1f cm (<10cm)\n", state.distance);
        Serial.println("🔴 LED: MENYALA");
        Serial.println("🔊 Buzzer: BERBUNYI");
        Serial.println("════════════════════════════════════════\n");
        state.wasAlertActive = true;
    } 
    else if (!shouldActivate && state.distance < 400 && (state.ledOn || state.buzzerOn)) {
        setDevices(false, false);
        Serial.println("╔════════════════════════════════════════╗");
        Serial.println("║           ✅ OBJEK AMAN!               ║");
        Serial.println("╚════════════════════════════════════════╝");
        Serial.printf("✓ Berjarak lebih dari 10 cm (%.1f cm)\n", state.distance);
        Serial.println("⚪ LED: MATI");
        Serial.println("🔇 Buzzer: MATI");
        Serial.println("════════════════════════════════════════\n");
        state.wasAlertActive = false;
    }
}

// ═══════════════════ JSON API ═══════════════════
void handleData() {
    StaticJsonDocument<512> doc;
    
    doc["distance"] = state.distance > 0 ? state.distance : 0;
    doc["minDistance"] = state.minDistance;
    doc["maxDistance"] = state.maxDistance;
    doc["avgDistance"] = state.avgDistance;
    doc["ledOn"] = state.ledOn;
    doc["buzzerOn"] = state.buzzerOn;
    doc["autoMode"] = state.autoMode;
    doc["measurementCount"] = state.measurementCount;
    doc["uptime"] = millis() / 1000;
    doc["rssi"] = WiFi.RSSI();
    doc["isAlert"] = (state.distance > 0 && state.distance <= DISTANCE_THRESHOLD);
    
    String response;
    serializeJson(doc, response);
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
}

// ═══════════════════ PREMIUM HTML DASHBOARD ═══════════════════
String generateHTML() {
    String html = F("<!DOCTYPE html><html><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1.0,maximum-scale=1.0,user-scalable=no'>"
        "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'>"
        "<title>ESP32 Ultra Dashboard</title><style>"
        "*{margin:0;padding:0;box-sizing:border-box;-webkit-tap-highlight-color:transparent}"
        "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,'Helvetica Neue',Arial,sans-serif;"
        "background:#0f0f23;color:#fff;overflow-x:hidden;-webkit-font-smoothing:antialiased;"
        "touch-action:manipulation}"
        
        ".bg-animated{position:fixed;top:0;left:0;width:100%;height:100%;z-index:0;"
        "background:linear-gradient(125deg,#0f0f23,#1a1a3e,#0f0f23);background-size:400% 400%;"
        "animation:gradientShift 15s ease infinite}"
        ".stars{position:fixed;top:0;left:0;width:100%;height:100%;z-index:1;pointer-events:none}"
        ".star{position:absolute;width:2px;height:2px;background:#fff;border-radius:50%;"
        "animation:twinkle 3s infinite}"
        
        ".container{position:relative;z-index:10;max-width:1400px;margin:0 auto;padding:15px}"
        
        ".header{background:rgba(255,255,255,0.08);backdrop-filter:blur(20px);border-radius:20px;"
        "padding:20px 15px;margin-bottom:20px;border:1px solid rgba(255,255,255,0.15);"
        "box-shadow:0 10px 40px rgba(0,0,0,0.3),inset 0 1px 0 rgba(255,255,255,0.1)}"
        ".header h1{font-size:1.5rem;font-weight:900;background:linear-gradient(135deg,#667eea,#764ba2,#f093fb);"
        "background-clip:text;-webkit-background-clip:text;-webkit-text-fill-color:transparent;"
        "margin-bottom:8px;animation:shimmer 3s infinite;line-height:1.3}"
        ".header-subtitle{color:#a0aec0;font-size:0.85rem;font-weight:500;line-height:1.4}"
        ".status-badge{display:inline-flex;align-items:center;gap:8px;background:rgba(16,185,129,0.2);"
        "border:1px solid #10b981;padding:6px 12px;border-radius:15px;font-size:0.75rem;margin-top:12px}"
        ".pulse-dot{width:6px;height:6px;background:#10b981;border-radius:50%;animation:pulse 2s infinite}"
        
        ".dashboard{display:grid;grid-template-columns:1fr;gap:15px;margin-bottom:20px}"
        
        ".card{background:rgba(255,255,255,0.08);backdrop-filter:blur(20px);border-radius:20px;"
        "padding:20px;border:1px solid rgba(255,255,255,0.15);position:relative;overflow:hidden;"
        "transition:all 0.3s cubic-bezier(0.4,0,0.2,1);box-shadow:0 4px 15px rgba(0,0,0,0.2)}"
        ".card:active{transform:scale(0.98)}"
        
        ".card-icon{font-size:2.5rem;margin-bottom:15px;background:linear-gradient(135deg,#667eea,#764ba2);"
        "background-clip:text;-webkit-background-clip:text;-webkit-text-fill-color:transparent;"
        "filter:drop-shadow(0 2px 8px rgba(102,126,234,0.4))}"
        ".card-title{font-size:0.75rem;color:#a0aec0;text-transform:uppercase;letter-spacing:1.5px;"
        "margin-bottom:12px;font-weight:700}"
        
        ".distance-display{position:relative;width:180px;height:180px;margin:15px auto}"
        ".circular-progress{transform:rotate(-90deg)}"
        ".progress-bg{fill:none;stroke:rgba(255,255,255,0.1);stroke-width:10}"
        ".progress-bar{fill:none;stroke-width:10;stroke-linecap:round;"
        "transition:stroke-dashoffset 0.5s ease,stroke 0.5s ease;filter:drop-shadow(0 0 8px currentColor)}"
        ".distance-value{position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);"
        "text-align:center}"
        ".distance-number{font-size:2.5rem;font-weight:900;line-height:1;"
        "background:linear-gradient(135deg,#667eea,#764ba2);background-clip:text;"
        "-webkit-background-clip:text;-webkit-text-fill-color:transparent;transition:all 0.3s}"
        ".distance-unit{font-size:1rem;color:#a0aec0;font-weight:700;margin-top:5px}"
        
        ".stat-value{font-size:2rem;font-weight:800;margin:12px 0;color:#fff;line-height:1;transition:all 0.3s}"
        ".stat-label{font-size:0.75rem;color:#a0aec0;text-transform:uppercase;letter-spacing:0.5px}"
        ".stat-icon{font-size:1.5rem;opacity:0.2;position:absolute;right:15px;top:15px}"
        
        ".device-grid{display:grid;gap:12px;margin-top:15px}"
        ".device-item{display:flex;align-items:center;justify-content:space-between;"
        "background:rgba(255,255,255,0.05);padding:12px 15px;border-radius:12px;"
        "border:1px solid rgba(255,255,255,0.08);transition:all 0.2s}"
        ".device-item:active{background:rgba(255,255,255,0.12);transform:scale(0.98)}"
        ".device-label{display:flex;align-items:center;gap:10px;font-weight:600;font-size:0.9rem}"
        ".device-label i{font-size:1.1rem}"
        ".device-status{padding:5px 12px;border-radius:10px;font-size:0.7rem;font-weight:700;"
        "text-transform:uppercase;letter-spacing:0.8px;white-space:nowrap;transition:all 0.3s}"
        ".status-on{background:linear-gradient(135deg,#10b981,#059669);box-shadow:0 3px 12px rgba(16,185,129,0.4);"
        "animation:pulse 2s infinite}"
        ".status-off{background:rgba(100,116,139,0.3);color:#94a3b8}"
        
        ".control-panel{background:rgba(255,255,255,0.08);backdrop-filter:blur(20px);border-radius:20px;"
        "padding:25px 20px;border:1px solid rgba(255,255,255,0.15);"
        "box-shadow:0 4px 15px rgba(0,0,0,0.2)}"
        ".control-title{font-size:1.3rem;font-weight:800;margin-bottom:20px;color:#fff;"
        "display:flex;align-items:center;gap:10px}"
        ".btn-grid{display:grid;grid-template-columns:1fr 1fr;gap:12px}"
        ".btn{padding:16px 12px;font-size:0.85rem;font-weight:700;border:none;border-radius:15px;"
        "cursor:pointer;transition:all 0.2s;text-decoration:none;text-align:center;color:#fff;"
        "position:relative;overflow:hidden;text-transform:uppercase;letter-spacing:0.8px;"
        "display:flex;align-items:center;justify-content:center;gap:8px;min-height:54px;"
        "box-shadow:0 4px 15px rgba(0,0,0,0.2);touch-action:manipulation}"
        ".btn::before{content:'';position:absolute;top:50%;left:50%;width:0;height:0;"
        "border-radius:50%;background:rgba(255,255,255,0.3);transition:width 0.4s,height 0.4s;"
        "transform:translate(-50%,-50%);z-index:0}"
        ".btn:active::before{width:300px;height:300px}"
        ".btn:active{transform:scale(0.95)}"
        ".btn i{font-size:1.1rem;z-index:1}"
        ".btn span{z-index:1}"
        ".btn-on{background:linear-gradient(135deg,#10b981,#059669);box-shadow:0 6px 20px rgba(16,185,129,0.4)}"
        ".btn-on:active{box-shadow:0 3px 10px rgba(16,185,129,0.4)}"
        ".btn-off{background:linear-gradient(135deg,#ef4444,#dc2626);box-shadow:0 6px 20px rgba(239,68,68,0.4)}"
        ".btn-off:active{box-shadow:0 3px 10px rgba(239,68,68,0.4)}"
        ".btn-all{grid-column:1/-1;padding:18px 12px;font-size:0.95rem;min-height:60px}"
        
        ".alert-banner{background:linear-gradient(135deg,#ef4444,#dc2626);padding:15px 20px;"
        "border-radius:15px;margin-bottom:20px;display:flex;align-items:center;gap:12px;"
        "animation:shake 0.5s infinite,glow 1s infinite;box-shadow:0 8px 30px rgba(239,68,68,0.5);"
        "flex-wrap:wrap;transition:opacity 0.3s,transform 0.3s}"
        ".alert-icon{font-size:1.5rem;animation:rotate 2s linear infinite;flex-shrink:0}"
        ".alert-text{font-size:0.95rem;font-weight:700;text-transform:uppercase;letter-spacing:0.5px;"
        "line-height:1.4;flex:1;min-width:0}"
        ".alert-hidden{opacity:0;transform:translateY(-20px);height:0;overflow:hidden;padding:0;margin:0}"
        
        ".footer-stats{display:grid;grid-template-columns:repeat(2,1fr);gap:12px;margin-top:20px}"
        ".footer-stat{background:rgba(255,255,255,0.05);padding:15px;border-radius:12px;"
        "text-align:center;border:1px solid rgba(255,255,255,0.08)}"
        ".footer-stat-value{font-size:1.3rem;font-weight:800;color:#667eea;margin-bottom:5px;"
        "line-height:1;word-break:break-all;transition:all 0.3s}"
        ".footer-stat-label{font-size:0.65rem;color:#a0aec0;text-transform:uppercase;"
        "letter-spacing:0.5px;line-height:1.3}"
        
        "@keyframes gradientShift{0%,100%{background-position:0% 50%}50%{background-position:100% 50%}}"
        "@keyframes shimmer{0%,100%{background-position:0% 50%}50%{background-position:100% 50%}}"
        "@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.7}}"
        "@keyframes twinkle{0%,100%{opacity:0.3}50%{opacity:1}}"
        "@keyframes shake{0%,100%{transform:translateX(0)}25%{transform:translateX(-5px)}75%{transform:translateX(5px)}}"
        "@keyframes glow{0%,100%{box-shadow:0 8px 30px rgba(239,68,68,0.5)}"
        "50%{box-shadow:0 8px 50px rgba(239,68,68,0.8)}}"
        "@keyframes rotate{from{transform:rotate(0deg)}to{transform:rotate(360deg)}}"
        
        "@media(min-width:576px){"
        ".container{padding:20px}"
        ".header{padding:25px 20px;border-radius:22px}"
        ".header h1{font-size:1.8rem}"
        ".header-subtitle{font-size:0.95rem}"
        ".card{padding:25px;border-radius:22px}"
        ".card-icon{font-size:3rem}"
        ".distance-display{width:200px;height:200px}"
        ".distance-number{font-size:3rem}"
        ".btn-grid{gap:15px}"
        ".btn{font-size:0.9rem;padding:18px 15px}"
        ".btn i{font-size:1.2rem}"
        ".footer-stats{gap:15px}"
        ".footer-stat{padding:18px}"
        ".footer-stat-value{font-size:1.5rem}"
        ".footer-stat-label{font-size:0.7rem}"
        "}"
        
        "@media(min-width:768px){"
        ".container{padding:25px}"
        ".header{padding:30px;border-radius:25px}"
        ".header h1{font-size:2.2rem}"
        ".header-subtitle{font-size:1rem}"
        ".dashboard{grid-template-columns:repeat(2,1fr);gap:20px}"
        ".card{padding:30px;border-radius:25px}"
        ".card:hover{transform:translateY(-5px);box-shadow:0 20px 60px rgba(102,126,234,0.3)}"
        ".card-icon{font-size:3.5rem}"
        ".control-panel{padding:30px}"
        ".control-title{font-size:1.4rem}"
        ".btn-grid{gap:15px}"
        ".btn{font-size:0.95rem;min-height:56px}"
        ".btn-all{min-height:62px;font-size:1rem}"
        ".alert-banner{padding:18px 25px}"
        ".alert-text{font-size:1.1rem}"
        ".footer-stats{grid-template-columns:repeat(4,1fr);gap:18px}"
        ".footer-stat-value{font-size:1.6rem}"
        ".footer-stat-label{font-size:0.75rem}"
        "}"
        
        "@media(min-width:1024px){"
        ".header h1{font-size:2.5rem}"
        ".dashboard{grid-template-columns:repeat(3,1fr);gap:25px}"
        ".distance-display{width:220px;height:220px}"
        ".stat-value{font-size:2.5rem}"
        ".btn{font-size:1rem}"
        ".alert-banner{padding:20px 30px}"
        ".alert-text{font-size:1.2rem}"
        "}"
        
        "@media(max-height:500px)and(orientation:landscape){"
        ".header{padding:15px;margin-bottom:15px}"
        ".header h1{font-size:1.3rem}"
        ".dashboard{gap:12px}"
        ".card{padding:15px}"
        ".distance-display{width:140px;height:140px}"
        ".distance-number{font-size:2rem}"
        ".control-panel{padding:15px}"
        ".btn{padding:12px;min-height:48px;font-size:0.8rem}"
        ".footer-stats{gap:10px}"
        "}"
        "</style></head><body>"
        
        "<div class='bg-animated'></div>"
        "<div class='stars' id='stars'></div>"
        
        "<div class='container'>"
        "<div class='alert-banner alert-hidden' id='alertBanner'>"
        "<i class='fas fa-exclamation-triangle alert-icon'></i>"
        "<div class='alert-text'>⚠️ PERINGATAN! Objek Terdeteksi Terlalu Dekat!</div>"
        "</div>"
        
        "<div class='header'>"
        "<h1><i class='fas fa-microchip'></i> ESP32 DASHBOARD</h1>"
        "<div class='header-subtitle'>Next-Generation IoT Monitoring System</div>"
        "<div class='status-badge'><span class='pulse-dot'></span>Live Monitoring Active</div>"
        "</div>"
        
        "<div class='dashboard'>"
        
        "<div class='card'>"
        "<div class='card-icon'><i class='fas fa-radar'></i></div>"
        "<div class='card-title'>Distance Measurement</div>"
        "<div class='distance-display'>"
        "<svg class='circular-progress' width='100%' height='100%' viewBox='0 0 200 200'>"
        "<circle class='progress-bg' cx='100' cy='100' r='90'/>"
        "<circle class='progress-bar' id='progressBar' cx='100' cy='100' r='90' "
        "style='stroke:#667eea;stroke-dasharray:565;stroke-dashoffset:565'/>"
        "</svg>"
        "<div class='distance-value'>"
        "<div class='distance-number' id='distanceValue'>--</div>"
        "<div class='distance-unit'>CM</div></div></div></div>"
        
        "<div class='card'><i class='fas fa-chart-line stat-icon'></i>"
        "<div class='card-icon'><i class='fas fa-chart-bar'></i></div>"
        "<div class='card-title'>Statistics</div>"
        "<div style='display:grid;gap:12px'>"
        "<div><div class='stat-label'>Average Distance</div>"
        "<div class='stat-value' id='avgDistance'>0.0 <span style='font-size:1rem;color:#a0aec0'>cm</span></div></div>"
        "<div><div class='stat-label'>Min / Max Range</div>"
        "<div class='stat-value' id='minMaxDistance' style='font-size:1.3rem'>0 / 0 <span style='font-size:0.8rem;color:#a0aec0'>cm</span></div></div>"
        "</div></div>"
        
        "<div class='card'><i class='fas fa-microchip stat-icon'></i>"
        "<div class='card-icon'><i class='fas fa-server'></i></div>"
        "<div class='card-title'>Device Status</div>"
        "<div class='device-grid'>"
        "<div class='device-item'>"
        "<div class='device-label'><i class='fas fa-lightbulb'></i> LED</div>"
        "<div class='device-status' id='ledStatus'>○ OFF</div></div>"
        "<div class='device-item'>"
        "<div class='device-label'><i class='fas fa-volume-up'></i> Buzzer</div>"
        "<div class='device-status' id='buzzerStatus'>○ OFF</div></div>"
        "<div class='device-item'>"
        "<div class='device-label'><i class='fas fa-cog'></i> Mode</div>"
        "<div class='device-status' id='modeStatus' style='background:linear-gradient(135deg,#f59e0b,#d97706)'>AUTO</div>"
        "</div></div></div></div>"
        
        "<div class='control-panel'>"
        "<div class='control-title'><i class='fas fa-gamepad'></i> Control Panel</div>"
        "<div class='btn-grid'>"
        "<a href='/led/on' class='btn btn-on'><i class='fas fa-lightbulb'></i><span>LED ON</span></a>"
        "<a href='/led/off' class='btn btn-off'><i class='fas fa-lightbulb'></i><span>LED OFF</span></a>"
        "<a href='/buzzer/on' class='btn btn-on'><i class='fas fa-bell'></i><span>BUZZER ON</span></a>"
        "<a href='/buzzer/off' class='btn btn-off'><i class='fas fa-bell-slash'></i><span>BUZZER OFF</span></a>"
        "<a href='/all/on' class='btn btn-all btn-on'><i class='fas fa-power-off'></i><span>ACTIVATE ALL</span></a>"
        "<a href='/all/off' class='btn btn-all btn-off'><i class='fas fa-times-circle'></i><span>DEACTIVATE ALL</span></a>"
        "</div></div>"
        
        "<div class='footer-stats'>"
        "<div class='footer-stat'>"
        "<div class='footer-stat-value' id='measureCount'>0</div>"
        "<div class='footer-stat-label'>Measurements</div></div>"
        "<div class='footer-stat'>"
        "<div class='footer-stat-value' id='uptime'>0s</div>"
        "<div class='footer-stat-label'>Uptime</div></div>"
        "<div class='footer-stat'>"
        "<div class='footer-stat-value' style='font-size:1rem' id='ipAddress'>");
    html += WiFi.localIP().toString();
    html += F("</div><div class='footer-stat-label'>IP Address</div></div>"
        "<div class='footer-stat'>"
        "<div class='footer-stat-value' id='rssi'>0</div>"
        "<div class='footer-stat-label'>WiFi Signal (dBm)</div></div>"
        "</div>"
        "</div>"
        
        "<script>"
        "const stars=document.getElementById('stars');"
        "const starCount=window.innerWidth<768?30:50;"
        "for(let i=0;i<starCount;i++){"
        "const star=document.createElement('div');"
        "star.className='star';"
        "star.style.left=Math.random()*100+'%';"
        "star.style.top=Math.random()*100+'%';"
        "star.style.animationDelay=Math.random()*3+'s';"
        "stars.appendChild(star);"
        "}"
        
        "function updateData(){"
        "fetch('/data')"
        ".then(res=>res.json())"
        ".then(data=>{"
        "const dist=data.distance||0;"
        "const isAlert=data.isAlert||false;"
        
        "document.getElementById('distanceValue').textContent=dist>0?dist.toFixed(1):'--';"
        
        "const progressBar=document.getElementById('progressBar');"
        "const percent=dist>0?(dist/400):0;"
        "progressBar.style.strokeDashoffset=565-(565*percent);"
        "progressBar.style.stroke=isAlert?'#ef4444':'#667eea';"
        
        "const alertBanner=document.getElementById('alertBanner');"
        "if(isAlert){"
        "alertBanner.classList.remove('alert-hidden');"
        "}else{"
        "alertBanner.classList.add('alert-hidden');"
        "}"
        
        "document.getElementById('avgDistance').innerHTML=data.avgDistance.toFixed(1)+\"<span style='font-size:1rem;color:#a0aec0'> cm</span>\";"
        "document.getElementById('minMaxDistance').innerHTML=data.minDistance.toFixed(1)+' / '+data.maxDistance.toFixed(1)+\"<span style='font-size:0.8rem;color:#a0aec0'> cm</span>\";"
        
        "const ledStatus=document.getElementById('ledStatus');"
        "ledStatus.textContent=data.ledOn?'● ON':'○ OFF';"
        "ledStatus.className='device-status '+(data.ledOn?'status-on':'status-off');"
        
        "const buzzerStatus=document.getElementById('buzzerStatus');"
        "buzzerStatus.textContent=data.buzzerOn?'● ON':'○ OFF';"
        "buzzerStatus.className='device-status '+(data.buzzerOn?'status-on':'status-off');"
        
        "document.getElementById('modeStatus').textContent=data.autoMode?'AUTO':'MANUAL';"
        
        "document.getElementById('measureCount').textContent=data.measurementCount;"
        "document.getElementById('uptime').textContent=data.uptime+'s';"
        "document.getElementById('rssi').textContent=data.rssi;"
        "})"
        ".catch(err=>console.error('Error:',err));"
        "}"
        
        "updateData();"
        "setInterval(updateData,500);"
        "</script>"
        
        "</body></html>");
    
    return html;
}

// ═══════════════════ WEB HANDLERS ═══════════════════
void handleRoot() {
    server.send(200, "text/html", generateHTML());
}

void handleLED(bool turnOn) {
    setDevices(turnOn, state.buzzerOn);
    state.autoMode = false;
    Serial.printf("📱 Manual Control: LED %s\n", turnOn ? "ON" : "OFF");
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleBuzzer(bool turnOn) {
    setDevices(state.ledOn, turnOn);
    state.autoMode = false;
    Serial.printf("📱 Manual Control: Buzzer %s\n", turnOn ? "ON" : "OFF");
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAll(bool turnOn) {
    setDevices(turnOn, turnOn);
    state.autoMode = !turnOn;
    Serial.printf("📱 Manual Control: All devices %s (Auto mode: %s)\n", 
                  turnOn ? "ON" : "OFF", 
                  state.autoMode ? "ENABLED" : "DISABLED");
    server.sendHeader("Location", "/");
    server.send(303);
}

// ═══════════════════ SYSTEM INIT ═══════════════════
void initPins() {
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    setDevices(false, false);
}

void playStartupSequence() {
    for (uint8_t i = 0; i < 3; i++) {
        digitalWrite(PIN_BUZZER, HIGH);
        digitalWrite(PIN_LED, HIGH);
        delay(100);
        digitalWrite(PIN_BUZZER, LOW);
        digitalWrite(PIN_LED, LOW);
        delay(100);
    }
}

bool connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("\n🔌 Menghubungkan ke WiFi");
    
    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
        attempts++;
    }
    Serial.println();
    
    digitalWrite(PIN_LED, LOW);
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║       ✅ WiFi Berhasil Terhubung!      ║");
        Serial.println("╚════════════════════════════════════════╝");
        Serial.printf("📶 SSID: %s\n", WIFI_SSID);
        Serial.printf("🌐 IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("📡 Signal Strength: %d dBm\n", WiFi.RSSI());
        Serial.println("════════════════════════════════════════");
        Serial.print("\n🔗 Buka Browser & Akses:\n   http://");
        Serial.println(WiFi.localIP());
        Serial.println("════════════════════════════════════════\n");
        return true;
    }
    
    Serial.println("\n❌ Gagal terhubung ke WiFi!");
    Serial.println("💡 Periksa SSID dan Password WiFi Anda");
    return false;
}

void setupWebServer() {
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/led/on", []() { handleLED(true); });
    server.on("/led/off", []() { handleLED(false); });
    server.on("/buzzer/on", []() { handleBuzzer(true); });
    server.on("/buzzer/off", []() { handleBuzzer(false); });
    server.on("/all/on", []() { handleAll(true); });
    server.on("/all/off", []() { handleAll(false); });
    server.begin();
    Serial.println("🌐 Web Server: AKTIF");
}

// ═══════════════════ SETUP ═══════════════════
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║   ESP32 ULTRA DASHBOARD v4.1          ║");
    Serial.println("║   AJAX Smooth Update Edition           ║");
    Serial.println("╚════════════════════════════════════════╝");
    
    Serial.println("\n🔧 Inisialisasi sistem...");
    initPins();
    Serial.println("✓ Pin konfigurasi selesai");
    
    playStartupSequence();
    Serial.println("✓ Hardware test selesai");
    
    if (!connectWiFi()) {
        Serial.println("\n⚠️  Masuk ke mode error - Buzzer akan berbunyi");
        while (true) {
            digitalWrite(PIN_BUZZER, HIGH);
            delay(200);
            digitalWrite(PIN_BUZZER, LOW);
            delay(800);
        }
    }
    
    playStartupSequence();
    
    if (MDNS.begin("esp32")) {
        Serial.println("🌐 mDNS: http://esp32.local (alternatif akses)");
    }
    
    setupWebServer();
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║       KONFIGURASI SISTEM               ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.printf("📏 Threshold Jarak: %.1f cm\n", DISTANCE_THRESHOLD);
    Serial.printf("⏱️  Interval Pengukuran: %lu ms\n", MEASURE_INTERVAL);
    Serial.printf("🌐 IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("📡 WiFi RSSI: %d dBm\n", WiFi.RSSI());
    Serial.println("════════════════════════════════════════");
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║          🚀 SISTEM SIAP!               ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println("\n📱 Akses dari perangkat manapun:");
    Serial.print("   🖥️  Desktop: http://");
    Serial.println(WiFi.localIP());
    Serial.print("   📱 Mobile: http://");
    Serial.println(WiFi.localIP());
    Serial.println("\n💡 Mode AUTO aktif - Sistem akan otomatis");
    Serial.println("   mengaktifkan alarm jika jarak ≤ 10cm");
    Serial.println("\n✨ Update otomatis setiap 500ms tanpa refresh!\n");
    Serial.println("════════════════════════════════════════\n");
}

// ═══════════════════ MAIN LOOP ═══════════════════
void loop() {
    server.handleClient();
    
    unsigned long currentTime = millis();
    if (currentTime - state.lastMeasurement >= MEASURE_INTERVAL) {
        state.lastMeasurement = currentTime;
        
        float newDistance = measureDistance();
        
        if (newDistance > 0) {
            state.distance = newDistance;
            handleAutoMode();
        }
    }
    
    delay(2);
}