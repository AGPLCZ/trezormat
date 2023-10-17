#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include "IPAddress.h"
#include <ArduinoJson.h>

const char* ssid = "ag";
const char* password = "dekujizawifi";
const char* host = "wallet.paralelnipolis.cz";
const int httpsPort = 443;
const char* apiKey = "**************";
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
}

void loop() {
  // Logika mincovníku
  if (millis() - lastInterruptTime > 1 && totalInterruptsCount > 0) {
    currentBalance += totalInterruptsCount;
    Serial.println("Aktuální zůstatek: " + String(currentBalance) + " Kc");
    totalInterruptsCount = 0;
  }

  // Čtení tlačítka
  buttonState10 = digitalRead(buttonPin10);
  if (buttonState10 == LOW && currentBalance >= requiredAmount) {
    String lnUrl = createWithdrawLink();
    Serial.println("LN URL: " + lnUrl);
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
