#define LED 2      // LED PIN
#define BUZZER 4   // BUZZER PIN

void setup() {
  // Initialize digital pin LED_BUILTIN as an output.
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

// The loop function runs over and over again forever
void loop() {
  digitalWrite(LED, HIGH);   // Turn the LED & BUZZER on (HIGH is the voltage level)
  digitalWrite(BUZZER, HIGH);
  delay(1000);                // Wait for a second
  digitalWrite(LED, LOW);    // Turn the LED & BUZZER off by making the voltage LOW
  digitalWrite(BUZZER, LOW);
  delay(1000);                // Wait for a second
}