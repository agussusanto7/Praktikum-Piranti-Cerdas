const int LED_PIN = 18;  // Pin untuk LED
const int SWITCH_PIN = 17;  // Pin untuk switch

int switchState = 0;  // Status switch saat ini
int lastSwitchState = 0;  // Status switch sebelumnya
int pressCount = 0;  // Hitungan jumlah penekanan switch

unsigned long previousMillis = 0;  // Waktu terakhir LED diperbarui
const long interval = 1000;  // Interval untuk berkedip (1 detik)

void setup() {
  Serial.begin(9600);  // Memulai komunikasi serial untuk debug
  pinMode(SWITCH_PIN, INPUT);  // Set pin switch sebagai input
  pinMode(LED_PIN, OUTPUT);  // Set pin LED sebagai output
}

void loop() {
  // Membaca status switch saat ini
  switchState = digitalRead(SWITCH_PIN);

  // Mengecek apakah switch ditekan (deteksi rising edge)
  if (switchState == HIGH && lastSwitchState == LOW) {
    pressCount++;  // Menambah jumlah penekanan switch
    Serial.print("Press count: ");  // Menampilkan jumlah penekanan ke serial monitor
    Serial.println(pressCount);
    delay(50);  // Penundaan untuk menghindari bouncing pada switch
  }

  // Menyimpan status switch untuk iterasi berikutnya
  lastSwitchState = switchState;

  // Bergantinya mode berdasarkan jumlah penekanan switch
  switch (pressCount % 3) {
    case 0:  // Mode 3: LED MATI
      digitalWrite(LED_PIN, LOW);  // Mematikan LED
      break;
    case 1:  // Mode 1: LED MENYALA
      digitalWrite(LED_PIN, HIGH);  // Menyalakan LED
      break;
    case 2:  // Mode 2: LED BERKEDIP
      unsigned long currentMillis = millis();  // Mengambil waktu sekarang
      if (currentMillis - previousMillis >= interval) {  // Mengecek apakah interval 1 detik telah tercapai
        previousMillis = currentMillis;  // Menyimpan waktu saat ini untuk perhitungan interval berikutnya
        // Mengganti status LED (menyalakan atau mematikan)
        int ledState = digitalRead(LED_PIN);  // Membaca status LED saat ini
        digitalWrite(LED_PIN, !ledState);  // Membalikkan status LED
      }
      break;
  }
}
