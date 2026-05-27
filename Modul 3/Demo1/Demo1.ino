#include <DHT.h>

// Konfigurasi DHT11
#define DHTPIN 4          // Pin data DHT11 terhubung ke GPIO 4
#define DHTTYPE DHT11     // Tipe sensor DHT11

// Konfigurasi LDR Analog
#define LDRPIN 34         // Pin analog LDR terhubung ke GPIO 34 (ADC)

// Inisialisasi sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// Variabel untuk timing
unsigned long previousMillis = 0;
const long interval = 2000;  // Interval pembacaan 2 detik

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(115200);
  
  // Inisialisasi sensor DHT11
  dht.begin();
  
  Serial.println("=== Sistem Logging Data Sensor ===");
  Serial.println("DHT11 + LDR Analog Sensor");
  Serial.println("Format: JSON");
  Serial.println("==================================\n");
  
  delay(2000); // Delay awal untuk stabilisasi sensor
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Cek apakah sudah waktunya membaca sensor
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Baca data dari DHT11
    float suhu = dht.readTemperature();        // Suhu dalam Celsius
    float kelembaban = dht.readHumidity();     // Kelembaban dalam %
    
    // Baca data dari LDR Analog
    int nilaiLDR = analogRead(LDRPIN);         // Nilai ADC (0-4095)
    // CATATAN: Pada modul LDR tertentu, logika terbalik:
    // 4095 = Gelap (sensor tertutup/tidak ada cahaya)
    // 0 = Terang (sensor terkena cahaya)
    
    // Inversi nilai agar logis (0=gelap, 4095=terang)
    int nilaiInversi = 4095 - nilaiLDR;
    float intensitasCahaya = (nilaiInversi / 4095.0) * 100;  // Konversi ke persentase
    
    String kondisiCahaya;
    if (nilaiInversi < 1000) {
      kondisiCahaya = "Gelap";
    } else if (nilaiInversi < 2500) {
      kondisiCahaya = "Redup";
    } else {
      kondisiCahaya = "Terang";
    }
    
    // Cek apakah pembacaan DHT11 berhasil
    if (isnan(suhu) || isnan(kelembaban)) {
      Serial.println("{\"error\": \"Gagal membaca sensor DHT11\"}");
      return;
    }
    
    // Tampilkan dalam format JSON
    Serial.println("{");
    Serial.print("  \"timestamp\": ");
    Serial.print(millis());
    Serial.println(",");
    
    Serial.println("  \"sensor\": {");
    
    Serial.println("    \"dht11\": {");
    Serial.print("      \"suhu\": ");
    Serial.print(suhu, 1);
    Serial.println(",");
    Serial.print("      \"kelembaban\": ");
    Serial.print(kelembaban, 1);
    Serial.println(",");
    Serial.println("      \"satuan\": \"°C / %\"");
    Serial.println("    },");
    
    Serial.println("    \"ldr\": {");
    Serial.print("      \"nilai_cahaya\": ");
    Serial.print(nilaiInversi);
    Serial.println(",");
    Serial.print("      \"intensitas_cahaya\": ");
    Serial.print(intensitasCahaya, 1);
    Serial.println(",");
    Serial.print("      \"kondisi\": \"");
    Serial.print(kondisiCahaya);
    Serial.println("\",");
    Serial.println("      \"satuan\": \"%\"");
    Serial.println("    }");
    
    Serial.println("  }");
    Serial.println("}\n");
    
    // Alternatif: Format Teks Sederhana (komen ini jika ingin pakai)
    /*
    Serial.println("=== Data Sensor ===");
    Serial.print("Suhu: ");
    Serial.print(suhu, 1);
    Serial.println(" °C");
    
    Serial.print("Kelembaban: ");
    Serial.print(kelembaban, 1);
    Serial.println(" %");
    
    Serial.print("Intensitas Cahaya: ");
    Serial.print(intensitasCahaya, 1);
    Serial.print(" % (");
    Serial.print(kondisiCahaya);
    Serial.print(") - Nilai: ");
    Serial.println(nilaiInversi);
    Serial.println("==================\n");
    */
  }
}