#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  delay(2000);
  Serial.println("start");
  // Odešleme zkušební zprávu na Serial2
  Serial2.println("Ahoj světe!");
}

void loop() {
  Serial2.println("Ahoj světe!");
  // Čekáme na odpověď z Serial2
  if (Serial2.available()) {
    // Zobrazíme odpověď na Serial
    Serial.println(Serial2.readString());
  }
}


#include <SoftwareSerial.h>

SoftwareSerial mySerial(9, 11); // RX, TX

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);
  delay(2000);
  Serial.println("start Uno");
  // Odešleme zkušební zprávu na mySerial
  mySerial.println("Ahoj od Uno!");
}

void loop() {
  // Čekáme na odpověď z mySerial
  if (mySerial.available()) {
    // Zobrazíme odpověď na Serial
    Serial.println(mySerial.readString());
  }

  // Odešleme zprávu každých pár sekund
  delay(9000);
  mySerial.println("Ahoj od Uno!");
}
