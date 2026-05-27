/*************************************************************
  Sistem Lampu Cerdas dengan NodeMCU ESP32, LDR, dan Relay
  Terhubung dengan Blynk IoT Platform
  
  Komponen:
  - NodeMCU ESP32
  - Sensor LDR (Light Dependent Resistor)
  - Relay Module
  - Lampu/Bohlam
  
  Koneksi Hardware (PENTING):
  - LDR: 
    * Kaki 1 LDR → 3.3V ESP32
    * Kaki 2 LDR → Pin 34 (ADC) DAN Resistor 10K
    * Resistor 10K → GND
  - Relay: Pin 26
  
*************************************************************/

// Library yang dibutuhkan
#define BLYNK_TEMPLATE_ID "TMPL6feriMh7f"
#define BLYNK_TEMPLATE_NAME "Smart Lamp"
#define BLYNK_AUTH_TOKEN "oPuJvociS6N4fao8Gk4Ykz-FiNr5KgZX"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Kredensial WiFi
char ssid[] = "Cristiano Ronaldo";
char pass[] = "Ronaldo12";

// Pin Definition
const int LDR_PIN = 34;        // Pin sensor LDR (ADC)
const int RELAY_PIN = 26;      // Pin relay

// Variabel
int ldrValue = 0;              // Nilai pembacaan LDR (raw ADC)
int ldrInverted = 0;           // Nilai LDR yang sudah dibalik
int threshold = 2000;          // Ambang batas terang/gelap
bool isAutoMode = true;        // Mode otomatis aktif
bool lampState = false;        // Status lampu
String lightStatus = "Terang"; // Status cahaya

// Virtual Pin Blynk
#define VPIN_LAMP_BUTTON V0    // Tombol kontrol manual
#define VPIN_LIGHT_STATUS V1   // Status cahaya (Terang/Gelap)
#define VPIN_LAMP_STATE V2     // Status lampu (Mati/Nyala)
#define VPIN_LDR_VALUE V3      // Nilai LDR
#define VPIN_AUTO_MODE V4      // Toggle mode otomatis

BlynkTimer timer;

void setup()
{
  Serial.begin(115200);
  
  // Setup pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relay mati (Active LOW: HIGH = OFF)
  
  // Koneksi ke Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Setup timer untuk membaca sensor setiap 1 detik
  timer.setInterval(1000L, readSensorAndControl);
  
  Serial.println("Sistem Lampu Cerdas Siap!");
  Serial.println("Format: LDR Raw | LDR Display | Cahaya | Lampu");
}

void loop()
{
  Blynk.run();
  timer.run();
}

// Fungsi membaca sensor dan kontrol otomatis
void readSensorAndControl()
{
  // Baca nilai LDR (raw)
  ldrValue = analogRead(LDR_PIN);
  
 
  ldrInverted = 4095 - ldrValue;
  
  
  if (ldrInverted > threshold) {
    lightStatus = "Terang";
    
    // Jika mode otomatis aktif, matikan lampu
    if (isAutoMode) {
      turnOffLamp();
    }
  } else {
    lightStatus = "Gelap";
    
    // Jika mode otomatis aktif, nyalakan lampu
    if (isAutoMode) {
      turnOnLamp();
    }
  }
  
  // Kirim data ke Blynk (kirim nilai yang sudah dibalik)
  Blynk.virtualWrite(VPIN_LDR_VALUE, ldrInverted);
  Blynk.virtualWrite(VPIN_LIGHT_STATUS, lightStatus);
  Blynk.virtualWrite(VPIN_LAMP_STATE, lampState ? "Nyala" : "Mati");
  
  // Debug Serial Monitor
  Serial.print("LDR Raw: ");
  Serial.print(ldrValue);
  Serial.print(" | LDR Display: ");
  Serial.print(ldrInverted);
  Serial.print(" | Cahaya: ");
  Serial.print(lightStatus);
  Serial.print(" | Lampu: ");
  Serial.println(lampState ? "Nyala" : "Mati");
}

// Fungsi menyalakan lampu
void turnOnLamp()
{
  if (!lampState) {
    digitalWrite(RELAY_PIN, LOW);  // Active LOW: LOW = Relay ON
    lampState = true;
    Blynk.virtualWrite(VPIN_LAMP_BUTTON, 1);
    Serial.println(">>> Lampu DINYALAKAN <<<");
  }
}

// Fungsi mematikan lampu
void turnOffLamp()
{
  if (lampState) {
    digitalWrite(RELAY_PIN, HIGH);  // Active LOW: HIGH = Relay OFF
    lampState = false;
    Blynk.virtualWrite(VPIN_LAMP_BUTTON, 0);
    Serial.println(">>> Lampu DIMATIKAN <<<");
  }
}

// Kontrol manual dari tombol Blynk (V0)
BLYNK_WRITE(VPIN_LAMP_BUTTON)
{
  int value = param.asInt();
  
  if (value == 1) {
    turnOnLamp();
  } else {
    turnOffLamp();
  }
}

// Toggle mode otomatis dari Blynk (V4)
BLYNK_WRITE(VPIN_AUTO_MODE)
{
  isAutoMode = param.asInt();
  
  if (isAutoMode) {
    Serial.println("Mode OTOMATIS Aktif");
  } else {
    Serial.println("Mode MANUAL Aktif");
  }
}

// Sinkronisasi status saat koneksi Blynk tersambung
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(VPIN_LAMP_BUTTON);
  Blynk.syncVirtual(VPIN_AUTO_MODE);
  Serial.println("Terhubung ke Blynk!");
}