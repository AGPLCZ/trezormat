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
      Serial.println("test"); // vypíše "test" do sériového monitoru
    }
  }



 if (swSerial.available()) { // zkontrolujte, zda jsou dostupná data ke čtení
    String message = swSerial.readStringUntil('\xFF'); // čtěte data až do 0xFF (konec zprávy)
    int testIndex = message.indexOf("TEST"); // najděte index "TEST" v přijaté zprávě
    if (testIndex >= 0) { // zkontrolujte, zda zpráva obsahuje "TEST"
      // Zde můžete přidat další logiku, pokud potřebujete zpracovat data před nebo za "TEST"
      Serial.println("test"); // vypíše "test" do sériového monitoru
    }
  }

  

}




// new
void setup() {
  Serial.begin(9600);
    pinMode(13  , OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH);  
  Serial.print("qr0.txt=");
  Serial.print('"');
  Serial.print("This is demo for QR code!!!");
  Serial.print('"');
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(2000);

  digitalWrite(13, LOW);   
  Serial.print("qr0.txt=");
  Serial.print('"');
  Serial.print("viola, got change after 2sec.");
  Serial.print('"');
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(2000);

  digitalWrite(13, HIGH);   
  Serial.print("qr0.txt=");
  Serial.print('"');
  Serial.print("ohh...yess...It is working....");
  Serial.print('"');
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(2000);

  digitalWrite(13, LOW);   
  Serial.print("qr0.txt=");
  Serial.print('"');
  Serial.print("abcdefghijklmnopqrstuvwxyz");
  Serial.print('"');
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(2000);

  digitalWrite(13, HIGH); 
  Serial.print("qr0.txt=");
  Serial.print('"');
  Serial.print("!@#$%^&*()_+=-");
  Serial.print('"');
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(2000);

  digitalWrite(13, LOW); 
  Serial.print("qr0.txt=");
  Serial.print('"');
  Serial.print("1234567890");
  Serial.print('"');
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(2000);

  
  
   
}

