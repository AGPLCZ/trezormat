const int pinDetekceMince = 2; // Pin, na který je připojen detektor mincí
const int pozadovanaCena = 75; // Cena, která musí být dosažena pro uvolnění zboží
volatile long celkovyPocetPreruseni = 0; // Celkový počet detekovaných mincí
unsigned long soucasnyZustatek = 0; // Aktuální nahromaděná hodnota mincí
unsigned long casPoslednihoPreruseni = 0; // Čas poslední detekované mince

void setup() {
  Serial.begin(9600); // Inicializace sériového portu s rychlostí 9600 baudů
  attachInterrupt(digitalPinToInterrupt(pinDetekceMince), preruseniPriVhozuMince, RISING); // Nastavení přerušení pro detekci mince
}

void loop() {
  // Zkontrolujeme, zda od posledního přerušení uplynul dostatečný čas a zda byla detekována nějaká mince
  if (millis() - casPoslednihoPreruseni > 1 && celkovyPocetPreruseni > 0) {
    soucasnyZustatek += celkovyPocetPreruseni; // Přičteme počet detekovaných mincí k současnému zůstatku
    if (soucasnyZustatek >= pozadovanaCena) { // Pokud je současný zůstatek větší nebo roven požadované ceně
      Serial.println("balanceReached"); // Pošleme zprávu, že požadovaná cena byla dosažena
      soucasnyZustatek = 0; // Vynulujeme současný zůstatek
    }
    Serial.println("Zustatek: " + String(soucasnyZustatek) + " Kc"); // Vypíšeme aktuální zůstatek
    celkovyPocetPreruseni = 0; // Vynulujeme počet detekovaných mincí
  }
}

void preruseniPriVhozuMince() {
  celkovyPocetPreruseni++; // Zaznamenáme novou detekovanou minci
  casPoslednihoPreruseni = millis(); // Aktualizujeme čas poslední detekce
}
