// Nextion -> nový display pro můj projekt - Nextion 2,8" NX3224T028 TFT LCD displej
// podobný projekt https://navody.dratek.cz/navody-k-produktum/lcd-displej-nextion-2.8.html

#include <SoftwareSerial.h>
// nastavení čísel propojovacích pinů
#define RX 6  // 6 Tx dislay
#define TX 7  // 7 Rx display
// vytvoření objektu swSerial z knihovny s nastavenými piny
SoftwareSerial swSerial(RX, TX);
int credit = 9;
String lnUrl = "LN url adresa";


void setup() {
  Serial.begin(9600);


  // display
  swSerial.begin(9600);
  swSerial.print(F("t0.txt=\"Celkem: "));
  swSerial.print(F("0"));
  swSerial.print(F(" Kc\""));
  swSerial.write(0xff);
  swSerial.write(0xff);
  swSerial.write(0xff);




  swSerial.print("qr0.txt=");  // Nastavení příkazu pro aktualizaci QR kódu
  swSerial.print('"');         // Otevření uvozovek
  swSerial.print(lnUrl);       // Váš text, který má být zakódován do QR kódu
  swSerial.print('"');         // Uzavření uvozovek
  swSerial.write(0xff);
  swSerial.write(0xff);
  swSerial.write(0xff);  // Tři bajty 0xFF signalizují konec příkazu
}

void loop() {
  credit++;  // Zvýšení hodnoty credit o 1

  swSerial.print(F("t0.txt=\"Celkem: "));
  swSerial.print(String(credit));
  swSerial.print(F(" Kc\""));
  swSerial.write(0xff);
  swSerial.write(0xff);
  swSerial.write(0xff);



  if (swSerial.available()) { // zkontrolujte, zda jsou dostupná data ke čtení
    String message = swSerial.readStringUntil('\xFF'); // čtěte data až do 0xFF (konec zprávy)
    if (message.indexOf("b0") >= 0) { // zkontrolujte, zda zpráva obsahuje "TEST"
      credit = 0;
    }
  }

}
