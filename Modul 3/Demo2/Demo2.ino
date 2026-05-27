// Konfigurasi Pin LED dan Buzzer
#define LED_PIN 4        // LED terhubung ke GPIO 4
#define BUZZER_PIN 18     // Buzzer terhubung ke GPIO 18

// Konfigurasi Pin HC-SR04
#define TRIG_PIN 26       // Trigger HC-SR04 ke GPIO 26
#define ECHO_PIN 27       // Echo HC-SR04 ke GPIO 27

// Variabel untuk HC-SR04
long duration;
float distance;

// Variabel untuk timing pengukuran jarak
unsigned long previousMillis = 0;
const long interval = 500;  // Cek jarak setiap 500ms

// Status perangkat
bool ledStatus = false;
bool buzzerStatus = false;
bool autoMode = true;  // Mode otomatis untuk deteksi jarak

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(115200);
  
  // Inisialisasi pin LED dan Buzzer sebagai output
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Inisialisasi pin HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Matikan semua perangkat di awal
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("=== Sistem Kontrol LED & Buzzer ===");
  Serial.println("Perintah yang tersedia:");
  Serial.println("- LED ON    : Nyalakan LED");
  Serial.println("- LED OFF   : Matikan LED");
  Serial.println("- BUZZER ON : Nyalakan Buzzer");
  Serial.println("- BUZZER OFF: Matikan Buzzer");
  Serial.println("- ON        : Nyalakan semua");
  Serial.println("- OFF       : Matikan semua");
  Serial.println("===================================");
  Serial.println("HC-SR04: Deteksi otomatis aktif!");
  Serial.println("Jarak ≤ 10cm akan menyalakan LED & Buzzer\n");
}

void loop() {
  // Cek perintah dari Serial Monitor
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();  // Hapus spasi di awal/akhir
    command.toUpperCase();  // Ubah ke huruf besar
    
    processCommand(command);
  }
  
  // Pengukuran jarak secara berkala
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Ukur jarak
    distance = measureDistance();
    
    // Cek apakah objek terlalu dekat (≤ 10 cm)
    if (distance > 0 && distance <= 10) {
      // Objek terlalu dekat - Nyalakan LED & Buzzer otomatis
      if (!ledStatus || !buzzerStatus) {
        digitalWrite(LED_PIN, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        ledStatus = true;
        buzzerStatus = true;
        
        Serial.println("\n[PERINGATAN!] Objek terlalu dekat!");
        Serial.print("Jarak: ");
        Serial.print(distance, 1);
        Serial.println(" cm (<10cm)");
        Serial.println("LED & Buzzer: MENYALA OTOMATIS\n");
      }
    } else if (distance > 10 && distance < 400) {
      // Objek aman - Matikan jika dalam mode otomatis
      if (autoMode && (ledStatus || buzzerStatus)) {
        digitalWrite(LED_PIN, LOW);
        digitalWrite(BUZZER_PIN, LOW);
        ledStatus = false;
        buzzerStatus = false;
        
        Serial.println("\n[AMAN] Objek berjarak lebih dari 10 cm");
        Serial.print("Jarak: ");
        Serial.print(distance, 1);
        Serial.println(" cm");
        Serial.println("LED & Buzzer: MATI OTOMATIS\n");
      }
    }
  }
}

// Fungsi untuk mengukur jarak dengan HC-SR04
float measureDistance() {
  // Kirim pulsa trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Baca pulsa echo
  duration = pulseIn(ECHO_PIN, HIGH, 30000);  // Timeout 30ms
  
  // Hitung jarak (cm)
  // Rumus: jarak = (waktu × kecepatan suara) / 2
  // Kecepatan suara = 0.034 cm/μs
  float dist = (duration * 0.034) / 2;
  
  return dist;
}

// Fungsi untuk memproses perintah dari Serial Monitor
void processCommand(String cmd) {
  Serial.print("Perintah diterima: ");
  Serial.println(cmd);
  
  if (cmd == "LED ON") {
    digitalWrite(LED_PIN, HIGH);
    ledStatus = true;
    autoMode = false;  // Nonaktifkan mode otomatis
    Serial.println("→ LED: MENYALA");
    
  } else if (cmd == "LED OFF") {
    digitalWrite(LED_PIN, LOW);
    ledStatus = false;
    autoMode = false;
    Serial.println("→ LED: MATI");
    
  } else if (cmd == "BUZZER ON") {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerStatus = true;
    autoMode = false;
    Serial.println("→ BUZZER: MENYALA");
    
  } else if (cmd == "BUZZER OFF") {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerStatus = false;
    autoMode = false;
    Serial.println("→ BUZZER: MATI");
    
  } else if (cmd == "ON") {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    ledStatus = true;
    buzzerStatus = true;
    autoMode = false;
    Serial.println("→ SEMUA PERANGKAT: MENYALA");
    
  } else if (cmd == "OFF") {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    ledStatus = false;
    buzzerStatus = false;
    autoMode = true;  // Aktifkan kembali mode otomatis
    Serial.println("→ SEMUA PERANGKAT: MATI");
    Serial.println("→ Mode otomatis HC-SR04: AKTIF");
    
  } else {
    Serial.println("→ Perintah tidak dikenal!");
    Serial.println("   Gunakan: LED ON/OFF, BUZZER ON/OFF, ON, OFF");
  }
  
  Serial.println();
}
