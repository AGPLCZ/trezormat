// Nextion -> nový display pro můj projekt - Nextion 2,8" NX3224T028 TFT LCD displej
// podobný projekt https://navody.dratek.cz/navody-k-produktum/lcd-displej-nextion-2.8.html 

#include <SoftwareSerial.h>
// nastavení čísel propojovacích pinů
#define RX 6  // 6 Tx dislay
#define TX 7  // 7 Rx display
// vytvoření objektu swSerial z knihovny s nastavenými piny
SoftwareSerial swSerial(RX, TX);
int credit = 9;

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
}



void loop() {
    credit++;  // Zvýšení hodnoty credit o 1

  swSerial.print(F("t0.txt=\"Celkem: "));
  swSerial.print(String(credit));
  swSerial.print(F(" Kc\""));
  swSerial.write(0xff);
  swSerial.write(0xff);
  swSerial.write(0xff);
}
