// Deklarasi pin
int sensorPin = 34;   // Pin untuk Analog Output (AO) pada ESP32
int ledPin = 2;       // Pin untuk LED
int digitalPin = 13;  // Pin untuk Digital Output (DO) pada ESP32
int sensorValue = 0;  // Variabel untuk membaca nilai analog dari sensor
int digitalValue = 0; // Variabel untuk membaca nilai digital (HIGH/LOW)

void setup() {
  // Setup pin mode
  pinMode(ledPin, OUTPUT);    // Set pin LED sebagai output
  pinMode(digitalPin, INPUT); // Set pin Digital Output (DO) sebagai input
  Serial.begin(115200);       // Memulai komunikasi serial dengan baud rate 115200

  // Tampilkan pesan awal pada serial monitor
  Serial.println("MQ Sensor dengan ESP32");
}

void loop() {
  // Membaca nilai analog dari AO (sensor)
  sensorValue = analogRead(sensorPin);

  // Membaca nilai digital dari DO (sensor)
  digitalValue = digitalRead(digitalPin);

  // Menampilkan nilai pembacaan ke Serial Monitor
  Serial.print("Nilai Analog (AO): ");
  Serial.println(sensorValue);
  
  Serial.print("Status Digital (DO): ");
  if (digitalValue == HIGH) {
    Serial.println("tidak ada cahaya!");
    digitalWrite(ledPin, HIGH); // Menyalakan LED jika gas terdeteksi
  } else {
    Serial.println("cahaya terdeteksi ");
    digitalWrite(ledPin, LOW);  // Mematikan LED jika tidak ada gas
  }

  // Delay untuk pembacaan yang stabil
  delay(1000); // Delay 1 detik
}
