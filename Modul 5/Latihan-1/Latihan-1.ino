/*************************************************************
  Program: Monitoring Sensor LDR dengan Blynk IoT
  Board: NodeMCU ESP32
  Sensor: LDR (Light Dependent Resistor)
  
  Koneksi Hardware:
  - LDR Pin 1 → 3.3V
  - LDR Pin 2 → GPIO 34 (VP) → Resistor 10kΩ → GND
*************************************************************/

#define BLYNK_TEMPLATE_ID "TMPL6AH8OCnPK"
#define BLYNK_TEMPLATE_NAME "ldr"
#define BLYNK_AUTH_TOKEN "kcdaMGOi4GRNUyd8WOkiMVtUBZwyf3GN"

// Uncomment untuk debug
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Kredensial WiFi
char ssid[] = "OPPO A5 2020";     // Ganti dengan nama WiFi
char pass[] = "sekmarine"; // Ganti dengan password WiFi

// Pin sensor LDR
const int LDR_PIN = 34; // GPIO 34 (ADC1_CH6)

// Variabel untuk menyimpan nilai sensor
int nilaiLDR = 0;
int intensitasCahaya = 0;

// Timer untuk mengirim data
BlynkTimer timer;

// Fungsi untuk membaca sensor LDR dan mengirim ke Blynk
void bacaSensorLDR() {
  // Baca nilai analog dari LDR (0-4095 untuk ESP32)
  nilaiLDR = analogRead(LDR_PIN);
  
  // Balik nilai agar gelap = rendah, terang = tinggi
  nilaiLDR = 4095 - nilaiLDR;
  
  // Konversi ke persentase intensitas cahaya (0-100%)
  intensitasCahaya = map(nilaiLDR, 0, 4095, 0, 100);
  
  // Kirim data ke Blynk Virtual Pin
  Blynk.virtualWrite(V0, nilaiLDR);          // Nilai mentah ADC
  Blynk.virtualWrite(V1, intensitasCahaya);  // Persentase intensitas
  
  // Tampilkan di Serial Monitor
  Serial.println("===== Data Sensor LDR =====");
  Serial.print("Nilai ADC: ");
  Serial.println(nilaiLDR);
  Serial.print("Intensitas Cahaya: ");
  Serial.print(intensitasCahaya);
  Serial.println("%");
  Serial.println("===========================");
}

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n===== SISTEM MONITORING LDR =====");
  Serial.println("Menghubungkan ke WiFi...");
  
  // Konfigurasi pin LDR sebagai input
  pinMode(LDR_PIN, INPUT);
  
  // Koneksi ke Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  Serial.println("Terhubung ke Blynk Cloud!");
  Serial.println("================================\n");
  
  // Set timer untuk membaca sensor setiap 2 detik
  timer.setInterval(2000L, bacaSensorLDR);
}

void loop() {
  Blynk.run();   // Jalankan Blynk
  timer.run();   // Jalankan timer
}

// Fungsi callback ketika terhubung ke Blynk
BLYNK_CONNECTED() {
  Serial.println("Perangkat terhubung ke Blynk Cloud!");
  
  // Sinkronisasi status awal
  Blynk.syncAll();
}

// Fungsi untuk membaca data dari Virtual Pin V0 (opsional)
BLYNK_WRITE(V0) {
  int value = param.asInt();
  Serial.print("Nilai dari V0: ");
  Serial.println(value);
}

// Fungsi untuk membaca data dari Virtual Pin V1 (opsional)
BLYNK_WRITE(V1) {
  int value = param.asInt();
  Serial.print("Nilai dari V1: ");
  Serial.println(value);
}