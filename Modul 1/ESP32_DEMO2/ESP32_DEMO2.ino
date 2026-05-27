const int LED_PIN = 18;  // Pin untuk LED
const int SWITCH_PIN = 17;  // Pin untuk switch

int pressCount = 0;  // Hitungan jumlah penekanan switch
unsigned long previousMillis = 0;  // Waktu terakhir LED diperbarui
const long interval = 1000;  // Interval untuk berkedip (1 detik)

void setup() {
  Serial.begin(9600);  // Memulai komunikasi serial untuk debug
  pinMode(SWITCH_PIN, INPUT);  // Set pin switch sebagai input
  pinMode(LED_PIN, OUTPUT);  // Set pin LED sebagai output
}

void loop() {
  static int lastSwitchState = LOW;  // Menyimpan status switch sebelumnya

  // Membaca status switch dan deteksi perubahan (rising edge)
  int switchState = digitalRead(SWITCH_PIN);
  if (switchState == HIGH && lastSwitchState == LOW) {
    pressCount++;
    Serial.print("Press count: ");
    Serial.println(pressCount);
    delay(500);  // Penundaan untuk menghindari bouncing pada switch
  }
  lastSwitchState = switchState;  // Menyimpan status switch untuk iterasi berikutnya

  // Bergantinya mode berdasarkan jumlah penekanan switch
  switch (pressCount % 3) {
    case 0:  // Mode 3: LED MATI
      digitalWrite(LED_PIN, LOW);
      break;
    case 1:  // Mode 1: LED MENYALA
      digitalWrite(LED_PIN, HIGH);
      break;
    case 2:  // Mode 2: LED BERKEDIP
      if (millis() - previousMillis >= interval) {
        previousMillis = millis();
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Membalikkan status LED
      }
      break;
  }
}
