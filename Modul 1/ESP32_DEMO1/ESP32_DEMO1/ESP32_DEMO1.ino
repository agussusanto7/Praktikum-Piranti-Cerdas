const int LED_PIN = 18;  // Pin LED
const int SWITCH_PIN = 17;  // Pin Switch

unsigned long previousMillis = 0;  // Waktu terakhir LED diperbarui
const long interval = 1000;  // Waktu untuk LED berkedip (1 detik)

void setup() {
  pinMode(SWITCH_PIN, INPUT);  // Set Switch sebagai input
  pinMode(LED_PIN, OUTPUT);  // Set LED sebagai output
}

void loop() {
  int switchState = digitalRead(SWITCH_PIN);  // Baca status switch
  
  // Kalau switch ditekan
  if (switchState == HIGH) {
    if (millis() - previousMillis >= interval) {  // Cek setiap 1 detik
      previousMillis = millis();  // Set waktu baru
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Balikkan status LED (hidup/mati)
    }
  } 
  // Kalau switch dilepas, matikan LED
  else {
    digitalWrite(LED_PIN, LOW);  // LED mati
  }
}
