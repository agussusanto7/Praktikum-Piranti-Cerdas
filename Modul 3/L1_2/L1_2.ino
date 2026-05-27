#define LED_PIN 5  // LED di GPIO5

String inputString = ""; 

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  if (Serial.available()) {
    inputString = Serial.readStringUntil('\n'); // baca input sampai Enter
    inputString.trim(); // hapus spasi/enter

    if (inputString == "ON") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED menyala");
    }
    else if (inputString == "OFF") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED mati");
    }
    else {
      Serial.println("Perintah tidak dikenali. Gunakan ON/OFF.");
    }
  }
}
