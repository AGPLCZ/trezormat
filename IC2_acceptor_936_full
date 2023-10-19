#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include "IPAddress.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <qrcode.h>


// display
// Konfigurace displeje SSD1306 (změřte šířku a výšku vašeho displeje)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
// ----

const char* ssid = "ag";
const char* password = "dekujizawifi";
const char* host = "wallet.paralelnipolis.cz";
const int httpsPort = 443;
const char* apiKey = "ec8f0243ba9f44dbb7f720e8568b4366";
WiFiSSLClient client;

// Proměnné pro mincovník
const int coinDetectionPin = 2;
const int requiredAmount = 5; // minimální částka
volatile long totalInterruptsCount = 0;
unsigned long currentBalance = 0;
unsigned long lastInterruptTime = 0;

// Proměnné pro tlačítko
const int buttonPin10 = 10;
int buttonState10 = HIGH;

void coinInsertInterrupt();  // Deklarace funkce zde

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin10, INPUT_PULLUP);  // Nastavení tlačítka
  attachInterrupt(digitalPinToInterrupt(coinDetectionPin), coinInsertInterrupt, RISING); // Detekce mincí

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Připojování k WiFi...");
  }
  Serial.println("Připojeno k WiFi!");

  // Inicializace displeje
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Adresa I2C může být 0x3C nebo 0x3D
    for (;;);  // Zacyklujte, pokud inicializace selže
  }
  display.clearDisplay();  // Vyčistěte displej
}


void loop() {
    display.clearDisplay();  // Vyčistěte displej

  // Logika mincovníku
  if (millis() - lastInterruptTime > 1 && totalInterruptsCount > 0) {
    currentBalance += totalInterruptsCount;
    Serial.println("Aktuální zůstatek: " + String(currentBalance) + " Kc");

    // Zobrazte text
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Celkem: " + String(currentBalance) + " Kc");
    display.display();  // Aktualizujte displej
  


    totalInterruptsCount = 0;
  }

  // Čtení tlačítka
  buttonState10 = digitalRead(buttonPin10);
  if (buttonState10 == LOW && currentBalance >= requiredAmount) {
    String lnUrl = createWithdrawLink();
    Serial.println("LN URL: " + lnUrl);

    // Generujte a zobrazte QR kód
QRCode qrcode;
uint8_t qrcodeData[qrcode_getBufferSize(6)];  // Změněná hodnota z 3 na 6
qrcode_initText(&qrcode, qrcodeData, 6, 0, lnUrl.c_str()); // text 

display.clearDisplay();  // Vyčistěte displej
for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
        if (qrcode_getModule(&qrcode, x, y)) {
            display.fillRect(x*2, y*2, 2, 2, SSD1306_WHITE);  // Zmenšený QR kód na 1x1 pixel
    }
  }
}
display.display();  // Aktualizujte displej

    delay(50000);  // Počkejte 50 sekund před další aktualizací

    currentBalance = 0; // Po generování resetujeme zůstatek
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
  doc["webhook_headers"] = "Content-Type: application/json";
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
    if (line.length() <= 2) { // Konec hlaviček (prázdný řádek)
      break;
    }
    Serial.println(line); // Vypíše hlavičky
  }

  String responseBody = client.readString();
  Serial.println("Tělo odpovědi:");
  Serial.println(responseBody); // Vypíše tělo odpovědi

  return extractLNURLFromResponse(responseBody);
}
