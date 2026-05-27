#include <DHT.h>

#define DHTPIN   4        // pin data DHT11 ke GPIO4
#define DHTTYPE  DHT11    // tipe sensor
#define LDR_PIN  34       // pin LDR ke GPIO34 (ADC ESP32)
const int ADC_MAX = 4095; // 12-bit ADC ESP32

DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;
const long interval = 3000;  // jeda 3 detik

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float suhu = dht.readTemperature();   // °C
    float kelembapan = dht.readHumidity(); // %
    int cahayaRaw = analogRead(LDR_PIN);   // 0..4095
    int cahayaInverted = ADC_MAX - cahayaRaw;

    if (isnan(suhu) || isnan(kelembapan)) {
      Serial.println("Gagal membaca sensor DHT11!");
    } else {
      Serial.print("Suhu: ");
      Serial.print(suhu, 2);
      Serial.print(" C, Kelembapan: ");
      Serial.print(kelembapan, 2);
      Serial.print(" %, Inverted Cahaya: ");
      Serial.println(cahayaInverted);
    }
  }
}