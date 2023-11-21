//Uno
#include <Wire.h>

String zpravaProMega = "s11";

void setup() {
  Wire.begin(); // Zahájení I2C jako master
  Serial.begin(9600); // Zahájení sériové komunikace pro ladění
}

void loop() {
  
  Wire.beginTransmission(8); // Začátek transakce s zařízením s adresou 8
  Wire.write(zpravaProMega.c_str()); // Odeslání zprávy
  Wire.endTransmission(); // Ukončení transakce

  delay(500); // Krátká pauza

  Wire.requestFrom(8, 10); // Požádáme o 10 bajtů dat od zařízení s adresou 8
  String prijataData = ""; // Vytvoříme řetězec pro uložení přijatých dat

  while (Wire.available()) { // Dokud jsou data dostupná
    char c = Wire.read(); // Čteme jeden bajt
    prijataData += c; // Přidáme bajt do řetězce
  }

  Serial.println(prijataData); // Vypíšeme přijatá data
  delay(100); // Čekáme 1 sekundu
}

//Mega
#include <Wire.h>

String prijataData = ""; // Proměnná pro uchování přijatých dat

void setup() {
  Wire.begin(8); // Zahájení I2C jako slave s adresou 8
  Wire.onReceive(receiveEvent); // Nastavení funkce pro přijetí dat
  Wire.onRequest(requestEvent); // Nastavení funkce pro zasílání dat
  Serial.begin(9600); // Zahájení sériové komunikace pro ladění
}

void loop() {
  delay(100); // Čekání pro snížení zatížení procesoru
}

void receiveEvent(int howMany) {
  prijataData = ""; // Vymazání předchozích dat
  while (Wire.available()) { // Dokud jsou data dostupná
    char c = Wire.read(); // Čteme jeden bajt
    prijataData += c; // Přidáme bajt do řetězce
  }
  Serial.println(prijataData); // Vypíšeme přijatá data
}

void requestEvent() {
  String odpoved = "Ahoj Uno!!";
  Wire.write(odpoved.c_str()); // Odešleme odpověď jako C-string
}
