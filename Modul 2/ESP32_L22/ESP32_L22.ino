#include <ESP32Servo.h>  // Ganti dari <Servo.h>

int servoPos = 0;  // posisi awal servo
bool mode = false; // mode toggle

Servo myServo;

void setup() {
  // ESP32Servo bisa menggunakan pin PWM mana saja
  myServo.attach(18); // servo di pin 18
  myServo.write(servoPos); // posisi awal
}

void loop() {
  mode = !mode; // toggle otomatis
  if (mode) {
    servoPos = 90; // posisi 90 derajat
  } else {
    servoPos = 0; // posisi 0 derajat
  }
  myServo.write(servoPos); // pergerakan servo
  delay(1000); // jeda 1 detik
}