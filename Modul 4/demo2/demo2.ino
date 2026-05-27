#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// ===== KONFIGURASI WiFi =====
const char* ssid = "OPPO A5 2020";
const char* password = "sekmarine";

// ===== KONFIGURASI Server =====
const char* serverName = "http://10.211.77.163/Demo2/input.php";

// ===== KONFIGURASI Sensor DHT11 =====
#define DHTPIN 4       // Pin data DHT11 ke GPIO 4
#define DHTTYPE DHT11  // Tipe sensor

DHT dht(DHTPIN, DHTTYPE);

// ===== TIMER =====
unsigned long lastTime = 0;
unsigned long timerDelay = 5000; // Kirim data setiap 5 detik

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=====================================");
  Serial.println("    SISTEM MONITORING DHT11");
  Serial.println("=====================================");
  
  // Inisialisasi sensor
  dht.begin();
  Serial.println("✓ Sensor DHT11 siap!");
  Serial.println();
  
  // Koneksi WiFi
  Serial.println("Menghubungkan ke WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Status: ");
  
  WiFi.begin(ssid, password);
  
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempt++;
    
    // Timeout setelah 30 detik (60 attempts)
    if (attempt > 60) {
      Serial.println("\n✗ Gagal terhubung ke WiFi!");
      Serial.println("Cek SSID dan Password, lalu restart ESP32");
      while(1); // Stop program
    }
  }
  
  Serial.println();
  Serial.println("=====================================");
  Serial.println("✓ WiFi Berhasil Terhubung!");
  Serial.println("=====================================");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.println("=====================================");
  Serial.println("Sistem siap mengirim data!");
  Serial.println("=====================================\n");
}

void loop() {
  // Cek apakah sudah waktunya kirim data
  if ((millis() - lastTime) > timerDelay) {
    
    // Baca data dari sensor
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    // Cek apakah pembacaan berhasil
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("ERROR: Gagal membaca sensor!");
      lastTime = millis();
      return;
    }
    
    // Tampilkan data di Serial Monitor
    Serial.println("=====================================");
    Serial.println("Data Sensor DHT11:");
    Serial.print("Suhu: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Kelembaban: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.println("=====================================");
    
    // Kirim data ke server
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      // Mulai koneksi ke server
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");
      
      // Buat data JSON
      String jsonData = "{\"temperature\":" + String(temperature) + 
                        ",\"humidity\":" + String(humidity) + "}";
      
      Serial.println("Mengirim data ke server...");
      Serial.print("Data JSON: ");
      Serial.println(jsonData);
      
      // Kirim data
      int httpResponseCode = http.POST(jsonData);
      
      // Cek hasil pengiriman
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("Response Code: ");
        Serial.println(httpResponseCode);
        Serial.print("Response Server: ");
        Serial.println(response);
        
        if (httpResponseCode == 200) {
          Serial.println("✓ Data berhasil dikirim!");
        }
      } else {
        Serial.print("✗ Gagal mengirim data. Error: ");
        Serial.println(httpResponseCode);
      }
      
      http.end();
    } else {
      Serial.println("✗ WiFi terputus!");
    }
    
    Serial.println();
    lastTime = millis();
  }
}