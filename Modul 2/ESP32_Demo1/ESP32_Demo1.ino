#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);

const int buzzerPin = 32;

// Batas gerakan (semakin kecil = semakin sensitif)
const float batasGerak = 1.0;

void setup() {
  Serial.begin(9600);
  
  // Inisialisasi I2C di pin 21 (SDA) dan 22 (SCL)
  Wire.begin(21, 22);
  mpu6050.begin();
  
  // Kalibrasi sensor (jangan digerakkan dulu!)
  Serial.println("Tunggu kalibrasi...");
  mpu6050.calcGyroOffsets(true);
  Serial.println("Siap! Coba gerakkan sensor.");
  
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
}

void loop() {
  mpu6050.update();
  
  // Baca kecepatan rotasi (gyroscope)
  float gerakX = abs(mpu6050.getGyroX());
  float gerakY = abs(mpu6050.getGyroY());
  float gerakZ = abs(mpu6050.getGyroZ());
  
  // Kalau ada gerakan --> buzzer bunyi
  if (gerakX > batasGerak || gerakY > batasGerak || gerakZ > batasGerak) {
    digitalWrite(buzzerPin, HIGH);
    Serial.println("🔔 GERAKAN TERDETEKSI!");
  } else {
    digitalWrite(buzzerPin, LOW);
    Serial.println("Diam...");
  }
  
  delay(1000);
}