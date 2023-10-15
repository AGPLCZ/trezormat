#define NoteAcceptor Serial3

/* 
 * Acceptor NV10  - Arduino Mega 2560
 * NV10 PIN 1 (RS232) Tx <-> Arduino Serial3 Rx 15
 * NV10 PIN 5 (RS232) Rx <-> Arduino Serial3 Tx 14
 * NV10 Top tier of pins - TOP PIN 1 -> Rx 15 TOP PIN 3 -> Tx 14
 * NV10 PIN 15 (top) <-> Arduino Vin - 12 V   
 * NV10 PIN 16 (down) <-> Arduino GND)  
 * NoteAcceptor.write(184);  //Acceptor enabled
 * NoteAcceptor.write(185); //Acceptor Disabled
 * NoteAcceptor.write(170);  //Enable serial escrow mode
 * NoteAcceptor.write(191); // Enable escrow timeout
 * NoteAcceptor.write(182); //Status
 * NoteAcceptor.write(173); //Decline
 */

int credit = 0;
int max = 700;
byte accepted = 0;

void bill(int banknoteValue) {
    take(banknoteValue);
    if (accepted == 1) {
        credit += banknoteValue;
        Serial.print("Celkem přijato: ");
        Serial.print(credit);
        Serial.println(" Kč");
        accepted = 0;
    } else {
        accepted = 1;
    }
}

void take(int banknoteValue) {
  if ((credit + banknoteValue) <= max) {
    NoteAcceptor.write(172);  // Přijmout
  } else {
    NoteAcceptor.write(173);  // Vrátit
  }
}


void setup() {
  Serial.begin(9600);
  NoteAcceptor.begin(300, SERIAL_8N2);
  pinMode(15, INPUT_PULLUP);

  NoteAcceptor.write(184);  //Acceptor enabled
  NoteAcceptor.write(170);  //Enable serial escrow mode
  NoteAcceptor.write(191); // verifyAcceptor
  delay(1000);
}

void loop() {
  if (NoteAcceptor.available()) {
    byte byteIn = NoteAcceptor.read();
    Serial.print("Výstup: ");
    Serial.println(byteIn);

    if (byteIn == 70) {
      Serial.println("Přerušena úschova");
    } else if (byteIn == 1) {
      Serial.println("Přijato do akceptoru 100 Kč");
      bill(100);
    } else if (byteIn == 2) {
      Serial.println("Přijato do akceptoru 200 Kč");
      bill(200);
    } else if (byteIn == 3) {
      Serial.println("Přijato do akceptoru 300 Kč");
      bill(500);
    }
  }
}
