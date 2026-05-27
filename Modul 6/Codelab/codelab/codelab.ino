/*
 * SMART CAGE - PETERNAKAN AYAM PETELUR
 * Monitoring Suhu dengan DHT11 + Buzzer Alert + Blynk
 * 
 * FIX: Ganti pin buzzer ke GPIO 25 (aman untuk PWM)
 * 
 * Ketentuan:
 * - Suhu >= 35°C (PANAS) -> Buzzer KENCANG (beep cepat)
 * - Suhu 28-34°C (HANGAT) -> Buzzer sedang (beep lambat)
 * - Suhu < 28°C (NORMAL) -> Buzzer mati
 */

#define BLYNK_TEMPLATE_ID "TMPL6J0XhZm-J"
#define BLYNK_TEMPLATE_NAME "CodelabModul6"
#define BLYNK_AUTH_TOKEN "x9GMjslkCIqTm6RRX7eKcL3Urlr-JiSO"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Konfigurasi WiFi
char ssid[] = "OPPO A5 2020";
char pass[] = "sektalah";

// Pin Configuration
#define DHTPIN 4          // Pin DHT11 (GPIO 4)
#define DHTTYPE DHT11     // Tipe sensor DHT11
#define BUZZER_PIN 25     // Pin Buzzer (GPIO 25) - GANTI DARI GPIO 5!

// Inisialisasi DHT
DHT dht(DHTPIN, DHTTYPE);

// Blynk Virtual Pins
#define VPIN_TEMPERATURE V0   // Gauge suhu
#define VPIN_HUMIDITY V1      // Gauge kelembaban
#define VPIN_STATUS V2        // Label status

// Variabel global
float temperature = 0;
float humidity = 0;
String statusKandang = "NORMAL";

// Timer Blynk
BlynkTimer timer;

// Variabel untuk kontrol buzzer non-blocking
unsigned long lastBuzzerTime = 0;
int buzzerState = 0;
int buzzerMode = 0; // 0=mati, 1=sedang, 2=kencang
int beepCount = 0;  // Counter untuk beep berulang

void setup() {
  Serial.begin(115200);
  
  // Setup Pin Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Inisialisasi DHT
  dht.begin();
  
  // Koneksi ke Blynk
  Serial.println("Menghubungkan ke WiFi...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Tampilkan IP Address
  Serial.println("\n========================================");
  Serial.println("  SMART CAGE SYSTEM STARTED!");
  Serial.println("  Buzzer Pin: GPIO 25");
  Serial.print("  WiFi Connected! IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("========================================\n");
  
  // Set timer untuk membaca sensor setiap 2 detik
  timer.setInterval(2000L, readSensorAndControl);
}

void loop() {
  Blynk.run();
  timer.run();
  
  // Handle buzzer dengan non-blocking
  handleBuzzer();
}

// Fungsi membaca sensor dan kontrol buzzer
void readSensorAndControl() {
  // Baca data sensor
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  
  // Cek jika pembacaan gagal
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("❌ Error: Gagal membaca sensor DHT!");
    return;
  }
  
  // Tampilkan di Serial Monitor
  Serial.println("=============================");
  Serial.print("🌡️  Suhu: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("💧 Kelembaban: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  // Logika kontrol berdasarkan suhu
  if (temperature >= 35) {
    // SUHU PANAS EKSTREM - Buzzer KENCANG
    statusKandang = "🔥 PANAS! NYALAKAN KIPAS SEKARANG!";
    buzzerMode = 2; // Mode kencang
    Serial.println("🚨 Status: PANAS EKSTREM! Buzzer KENCANG");
    
  } else if (temperature >= 30 && temperature < 35) {
    // SUHU HANGAT - Buzzer sedang
    statusKandang = "⚠️ HANGAT - Segera nyalakan kipas";
    buzzerMode = 1; // Mode sedang
    Serial.println("⚠️  Status: HANGAT - Buzzer SEDANG");
    
  } else {
    // SUHU NORMAL - Buzzer mati
    statusKandang = "✅ NORMAL - Kondisi aman";
    buzzerMode = 0; // Mode mati
    Serial.println("✅ Status: NORMAL - Buzzer MATI");
  }
  
  Serial.println("=============================\n");
  
  // Kirim data ke Blynk
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature);
  Blynk.virtualWrite(VPIN_HUMIDITY, humidity);
  Blynk.virtualWrite(VPIN_STATUS, statusKandang);
}

// Fungsi handle buzzer non-blocking
void handleBuzzer() {
  unsigned long currentTime = millis();
  
  if (buzzerMode == 2) {
    // ===== MODE KENCANG (Panas Ekstrem) =====
    // Pattern: 3x beep cepat, pause, repeat
    // BEEP-BEEP-BEEP ... BEEP-BEEP-BEEP
    
    if (buzzerState == 0 && currentTime - lastBuzzerTime >= 100) {
      // Nyalakan buzzer
      tone(BUZZER_PIN, 100); // Frekuensi SANGAT tinggi
      buzzerState = 1;
      lastBuzzerTime = currentTime;
      beepCount++;
      Serial.println("🔊 BEEP! (Mode KENCANG)");
    } 
    else if (buzzerState == 1 && currentTime - lastBuzzerTime >= 150) {
      // Matikan buzzer
      noTone(BUZZER_PIN);
      buzzerState = 0;
      lastBuzzerTime = currentTime;
      
      // Reset counter setelah 3 beep, pause lebih lama
      if (beepCount >= 3) {
        beepCount = 0;
        lastBuzzerTime = currentTime + 300; // Pause 300ms
      }
    }
    
  } else if (buzzerMode == 1) {
    // ===== MODE SEDANG (Hangat) =====
    // Pattern: 1x beep panjang, pause lama
    // BEEEEEP ... BEEEEEP
    
    if (buzzerState == 0 && currentTime - lastBuzzerTime >= 1500) {
      // Nyalakan buzzer
      tone(BUZZER_PIN, 1700); // Frekuensi medium
      buzzerState = 1;
      lastBuzzerTime = currentTime;
      Serial.println("🔔 BEEP (Mode SEDANG)");
    } 
    else if (buzzerState == 1 && currentTime - lastBuzzerTime >= 600) {
      // Matikan buzzer
      noTone(BUZZER_PIN);
      buzzerState = 0;
      lastBuzzerTime = currentTime;
    }
    
  } else {
    // ===== MODE MATI (Normal) =====
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
    buzzerState = 0;
    beepCount = 0;
  }
}

// Callback saat ESP32 terhubung ke Blynk
BLYNK_CONNECTED() {
  Serial.println("✅ Terhubung ke Blynk Cloud!");
  Blynk.virtualWrite(VPIN_STATUS, "Sistem Online");
}