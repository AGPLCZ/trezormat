#include <SoftwareSerial.h>
#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include "IPAddress.h"
#include <ArduinoJson.h>
#include <Wire.h>

// nastavení čísel propojovacích pinů pro displej Nextion
#define RX 6  // 6 Tx dislay
#define TX 7  // 7 Rx display
// vytvoření objektu swSerial z knihovny s nastavenými piny
SoftwareSerial swSerial(RX, TX);

const char* ssid = "ag";
const char* password = "dekujizawifi";
const char* host = "legend.lnbits.com";
const int httpsPort = 443;
const char* apiKey = "5f9940a3227d4df0904db50603e4d041";
WiFiSSLClient client;

// Proměnné pro mincovník
const int coinDetectionPin = 2;
const int requiredAmount = 5;  // minimální částka
volatile long totalInterruptsCount = 0;
unsigned long currentBalance = 0;
unsigned long lastInterruptTime = 0;
String lnUrl_info = "LNURL1DP68GURN8GHJ7MR9VAJKUEPWD3HXY6T5WVHXXMMD9AMKJARGV3EXZAE0V9CXJTMKXYHKCMN4WFKZ7V6E8P3HW3TWG9VHXVM28PN56AEEGF2XWWTY9UMYY3NJX3QH55ZJ2EP9SWTGGFPNWVMDX42NGRX06FZ";
// Proměnné pro tlačítko
const int buttonPin10 = 10;
int buttonState10 = HIGH;

void coinInsertInterrupt();  // Deklarace funkce zde

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin10, INPUT_PULLUP);                                                     // Nastavení tlačítka
  attachInterrupt(digitalPinToInterrupt(coinDetectionPin), coinInsertInterrupt, RISING);  // Detekce mincí

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Připojování k WiFi...");
  }
  Serial.println("Připojeno k WiFi!");

  // Inicializace displeje Nextion
  swSerial.begin(9600);
  swSerial.print(F("t0.txt=\"Celkem: "));
  swSerial.print(F("0"));
  swSerial.print(F(" Kc\""));
  swSerial.write(0xff);
  swSerial.write(0xff);
  swSerial.write(0xff);



  swSerial.print("qr0.txt=");  // Nastavení příkazu pro aktualizaci QR kódu
  swSerial.print('"');         // Otevření uvozovek
  swSerial.print(lnUrl_info);       // Váš text, který má být zakódován do QR kódu
  swSerial.print('"');         // Uzavření uvozovek
  swSerial.write(0xff);
  swSerial.write(0xff);
  swSerial.write(0xff);  // Tři bajty 0xFF signalizují konec příkazu
}

void loop() {
  // Logika mincovníku
  if (millis() - lastInterruptTime > 1 && totalInterruptsCount > 0) {
    currentBalance += totalInterruptsCount;
    Serial.println("Aktuální zůstatek: " + String(currentBalance) + " Kc");

    // Zobrazte text na displeji Nextion
    swSerial.print(F("t0.txt=\"Celkem: "));
    swSerial.print(String(currentBalance));
    swSerial.print(F(" Kc\""));
    swSerial.write(0xff);
    swSerial.write(0xff);
    swSerial.write(0xff);



    totalInterruptsCount = 0;
  }

  // Čtení tlačítka
  buttonState10 = digitalRead(buttonPin10);
  if (buttonState10 == LOW && currentBalance >= requiredAmount) {
    String lnUrl = createWithdrawLink();
    Serial.println("LN URL: " + lnUrl);


    swSerial.print("qr0.txt=");  // Nastavení příkazu pro aktualizaci QR kódu
    swSerial.print('"');         // Otevření uvozovek
    swSerial.print(lnUrl);       // Váš text, který má být zakódován do QR kódu
    swSerial.print('"');         // Uzavření uvozovek
    swSerial.write(0xff);
    swSerial.write(0xff);
    swSerial.write(0xff);  // Tři bajty 0xFF signalizují konec příkazu


    delay(50000);  // Počkejte 50 sekund před další aktualizací

    currentBalance = 0;  // Po generování resetujeme zůstatek
  }
}

void coinInsertInterrupt() {
  totalInterruptsCount++;
  lastInterruptTime = millis();
}

String extractLNURLFromResponse(String responseBody) {
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, responseBody);

  if (doc.containsKey("lnurl")) {
    return doc["lnurl"].as<String>();
  }

  return "LNURL nebyl nalezen.";
}

String createWithdrawLink() {
  if (!client.connect(host, httpsPort)) {
    return "Připojení se nezdařilo!";
  }

  DynamicJsonDocument doc(1024);
  doc["title"] = "Můj automatický odkaz";
  doc["min_withdrawable"] = currentBalance;
  doc["max_withdrawable"] = currentBalance;
  doc["uses"] = 1;
  doc["wait_time"] = 60;
  doc["is_unique"] = true;
  doc["webhook_url"] = "https://dobrodruzi.cz/";
  doc["webhook_headers"] = "{\"Content-Type\": \"application/json\"}";
  doc["webhook_body"] = "{\"amount\": 100}";
  doc["custom_url"] = "https://dobrodruzi.cz/";


  String json;
  serializeJson(doc, json);

  String request = "POST /withdraw/api/v1/links HTTP/1.1\r\n";
  request += "Host: " + String(host) + "\r\n";
  request += "X-Api-Key: " + String(apiKey) + "\r\n";
  request += "Content-Type: application/json\r\n";
  request += "Content-Length: " + String(json.length()) + "\r\n";
  request += "\r\n" + json;

  client.print(request);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line.length() <= 2) {  // Konec hlaviček (prázdný řádek)
      break;
    }
    Serial.println(line);  // Vypíše hlavičky
  }

  String responseBody = client.readString();
  Serial.println("Tělo odpovědi:");
  Serial.println(responseBody);  // Vypíše tělo odpovědi

  return extractLNURLFromResponse(responseBody);
}
