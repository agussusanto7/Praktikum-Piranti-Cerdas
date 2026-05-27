#include <Wire.h>
#include <MPU6050.h>

// Pin definitions
const int LDR_PIN = 34;      // Pin ADC untuk LDR
const int LED_PIN = 2;        // Pin untuk LED

// MPU6050 object
MPU6050 mpu;

// Variables untuk MPU6050
int16_t ax, ay, az;
int16_t gx, gy, gz;
float angleX, angleY;

// Offset kalibrasi (akan diisi saat setup)
float offsetX = 0;
float offsetY = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Setup LED pin
  pinMode(LED_PIN, OUTPUT);
  
  // Test LED terlebih dahulu
  Serial.println("Testing LED...");
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED test completed");
  
  // Initialize I2C communication
  Wire.begin();
  
  // Initialize MPU6050
  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  
  // Verify connection
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
    
    // KALIBRASI: Baca nilai saat sensor datar
    Serial.println("Calibrating... Letakkan sensor DATAR!");
    delay(2000);
    
    // Ambil rata-rata dari 10 pembacaan
    float sumX = 0, sumY = 0;
    for (int i = 0; i < 10; i++) {
      mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      sumX += ax / 1638.4;
      sumY += ay / 1638.4;
      delay(100);
    }
    offsetX = sumX / 10.0;
    offsetY = sumY / 10.0;
    
    Serial.print("Offset X: ");
    Serial.print(offsetX, 2);
    Serial.print(" | Offset Y: ");
    Serial.println(offsetY, 2);
    Serial.println("Calibration completed!");
  } else {
    Serial.println("MPU6050 connection failed");
  }
  
  Serial.println("=================================");
  Serial.println("Sistem Monitoring Sensor");
  Serial.println("=================================");
  delay(2000);
}

void loop() {
  // Baca nilai LDR dan BALIK nilainya
  int ldrRaw = analogRead(LDR_PIN);
  int ldrValue = 4095 - ldrRaw;  // BALIK NILAI: sekarang 0=gelap, 4095=terang
  
  // Baca data MPU6050
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // Konversi accelerometer data ke angle (dalam derajat) dengan offset
  angleX = (ax / 1638.4) - offsetX;  // Kurangi offset untuk akurasi
  angleY = (ay / 1638.4) - offsetY;  // Kurangi offset untuk akurasi
  
  // Tampilkan header
  Serial.println("\n--- Data Sensor ---");
  
  // Tampilkan nilai LDR
  Serial.print("Nilai LDR: ");
  Serial.print(ldrValue);
  Serial.println();
  
  // ========== ANALISIS KONDISI CAHAYA ==========
  Serial.print("Kondisi Cahaya: ");
  
  if (ldrValue >= 0 && ldrValue <= 50) {
    // Rentang 0-50: keadaan gelap - LED MENYALA
    Serial.println("keadaan gelap");
    digitalWrite(LED_PIN, HIGH);  // LED MENYALA
  } 
  else if (ldrValue >= 51 && ldrValue <= 500) {
    // Rentang 51-500: keadaan redup - LED MENYALA
    Serial.println("keadaan redup");
    digitalWrite(LED_PIN, HIGH);  // LED MENYALA
  } 
  else if (ldrValue >= 501 && ldrValue <= 2000) {
    // Rentang 501-2000: cahaya terang - LED MATI
    Serial.println("cahaya terang");
    digitalWrite(LED_PIN, LOW);  // LED MATI
  } 
  else if (ldrValue > 2000) {
    // Lebih dari 2000: sangat terang - LED MATI
    Serial.println("sangat terang");
    digitalWrite(LED_PIN, LOW);  // LED MATI
  }
  
  // Tampilkan status LED
  Serial.print("Status LED: ");
  Serial.println(digitalRead(LED_PIN) == HIGH ? "MENYALA" : "MATI");
  
  // Tampilkan nilai MPU6050
  Serial.print("Angle X: ");
  Serial.print(angleX, 2);
  Serial.print(" | Angle Y: ");
  Serial.println(angleY, 2);
  
  // ========== ANALISIS KEMIRINGAN SENSOR ==========
  Serial.print("Posisi Sensor: ");
  
  bool hasPositionX = false;
  bool hasPositionY = false;
  
  // Cek sumbu X (threshold dikurangi jadi 8 agar lebih sensitif)
  if (angleX > 8) {
    Serial.print("sensor miring kiri");
    hasPositionX = true;
  } 
  else if (angleX < -8) {
    Serial.print("sensor miring kanan");
    hasPositionX = true;
  }
  
  // Cek sumbu Y (threshold dikurangi jadi 8 agar lebih sensitif)
  if (angleY > 8) {
    if (hasPositionX) {
      Serial.print(" & ");
    }
    Serial.print("sensor miring depan");
    hasPositionY = true;
  } 
  else if (angleY < -8) {
    if (hasPositionX) {
      Serial.print(" & ");
    }
    Serial.print("sensor miring belakang");
    hasPositionY = true;
  }
  
  // Jika tidak ada kemiringan (sensor datar)
  if (!hasPositionX && !hasPositionY) {
    Serial.print("sensor posisi datar");
  }
  
  Serial.println();
  Serial.println("=================================");
  
  // Delay untuk pembacaan berikutnya
  delay(1000);
}