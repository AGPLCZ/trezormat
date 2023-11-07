#define NoteAcceptor Serial3  // Definujeme NoteAcceptor jako alias pro Serial3
#define Display Serial2       // Definujeme Display jako alias pro Serial2

int credit = 0;
int maxV = 700;
String lnUrl = "LN url adresa";
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
  if ((credit + banknoteValue) <= maxV) {
    NoteAcceptor.write(172);  // Přijmout
  } else {
    NoteAcceptor.write(173);  // Vrátit
  }
}

void setup() {
  delay(2000);
  
  Serial.begin(9600);
  Serial.print("start");
  
  // Inicializace hardwarových sériových portů
  NoteAcceptor.begin(300, SERIAL_8N2);
  Display.begin(9600);

  // Konfigurace pinů, pokud je potřeba
  pinMode(15, INPUT_PULLUP);

  // Nastavení zařízení
  NoteAcceptor.write(184);  // Acceptor enabled
  NoteAcceptor.write(170);  // Enable serial escrow mode
  NoteAcceptor.write(191);  // verifyAcceptor

  // Nastavení displeje
  Display.print(F("t0.txt=\"Vloženo: "));
  Display.print(F("0"));
  Display.print(F(" Kc\""));
  Display.write(0xff);
  Display.write(0xff);
  Display.write(0xff);
}

void loop() {
  // Kontrola příchozích dat od banknotového akceptoru
  if (NoteAcceptor.available()) {
    byte byteIn = NoteAcceptor.read();
    Serial.print("Výstup: ");
    Serial.println(byteIn);

    // Příklad zpracování příchozích dat
    if (byteIn == 70) {
      Serial.println("Přerušena úschova");
    } else if (byteIn == 1) {
      Serial.println("Přijato do akceptoru 100 Kč");
      bill(100);
    } else if (byteIn == 2) {
      Serial.println("Přijato do akceptoru 200 Kč");
      bill(200);
    } else if (byteIn == 3) {
      Serial.println("Přijato do akceptoru 500 Kč");
      bill(500);
    }
  }

  // Kontrola příchozích dat od displeje
  if (Display.available()) {
    String message = Display.readStringUntil('\xFF');  // Přečtení dat až po koncový znak
    if (message.indexOf("b0") >= 0) {  // Pokud je v zprávě obsaženo "b0"
      credit = 0;  // Reset kreditu na nulu
      Display.print("qr0.txt=\"");  // Aktualizace textu QR kódu
      Display.print(lnUrl);
      Display.print("\"");
      Display.write(0xff);
      Display.write(0xff);
      Display.write(0xff);
    }
  }

  // Aktualizace zobrazené hodnoty kreditu na displeji
  Display.print(F("t0.txt=\"Vloženo: "));
  Display.print(String(credit));
  Display.print(F(" Kc\""));
  Display.write(0xff);
  Display.write(0xff);
  Display.write(0xff);
}
