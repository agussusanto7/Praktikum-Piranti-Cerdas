#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Cristiano Ronaldo";         // Nama WiFi
const char* password = "Ronaldo12";             // Password WiFi
const char* server = "http://192.168.18.59/ldr_codelab/input.php";  // Ganti dengan IP lokal server PHP

const int ldrPin = 34; // LDR di GPIO34
int ldrValue = 0;

void setup() {
  Serial.begin(115200);  // Inisialisasi serial monitor
  WiFi.begin(ssid, password);  // Menghubungkan ESP32 ke WiFi
  Serial.println("Menghubungkan ke WiFi");

  while (WiFi.status() != WL_CONNECTED) {  // Menunggu hingga WiFi terhubung
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi Tersambung");
}

void loop() {
  // Baca nilai dari sensor LDR
  ldrValue = analogRead(ldrPin);
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);

  // Menentukan status cahaya hanya untuk ditampilkan di serial monitor
  if (ldrValue > 500) {
    Serial.println("Status: Cahaya Terang");
  } else {
    Serial.println("Status: Cahaya Redup");
  }

  // Kirim data LDR ke server dalam format JSON
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server);
    http.addHeader("Content-Type", "application/json");

    // Payload hanya mengandung nilai LDR
    String jsonPayload = "{\"ldr_value\":" + String(ldrValue) + "}";
    Serial.println("JSON Payload: " + jsonPayload);  // Debug payload

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server response: " + response);  // Menampilkan respons server
    } else {
      Serial.print("HTTP POST gagal. Error code: ");
      Serial.println(httpResponseCode);  // Jika gagal, tampilkan error code
    }

    http.end();  // Menutup koneksi HTTP
  }

  delay(15000);  // Delay agar tidak spam server
}
