#define LDR_PIN 34  // Define the analog pin connected to LDR

void setup() {
  Serial.begin(9600);
  pinMode(LDR_PIN, INPUT);  // Set LDR_PIN as input
}

void loop() {
  int analogValue = analogRead(LDR_PIN);  // Read the analog value from the LDR
  int invertedValue = 4095 - analogValue;  // Invert the reading to match light intensity
  
  Serial.print("Inverted LDR Value: ");
  Serial.println(invertedValue);  // Print the inverted LDR value to the Serial Monitor
  delay(1000);  // Wait for 1 second
}
