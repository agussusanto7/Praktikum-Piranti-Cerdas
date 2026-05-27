const int LED = 2;   // Pin LED terhubung ke pin digital 2

void setup() {
  Serial.begin(9600);     // Memulai komunikasi serial dengan baud rate 9600
  pinMode(LED, OUTPUT);   // Atur pin LED sebagai output
}

void loop() {
  digitalWrite(LED, HIGH);  // Nyalakan LED
  delay(3000);              // Tunggu 3 detik
  digitalWrite(LED, LOW);   // Matikan LED
  delay(3000);              // Tambahkan delay agar LED tidak langsung menyala lagi
}
