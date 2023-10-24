#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "ag";
const char* password = "dekujizawifi";
const char* host = "wallet.paralelnipolis.cz";
const int httpsPort = 443;
const char* apiKey = "ec8f0243ba9f44dbb7f720e8568b4366";
unsigned long currentBalance = 100;


const float btcToCzkRate = 663943.90;  // pevný kurz: 1 BTC = 663 943,90 CZK
const float satsPerCzk = 100000000.0 / btcToCzkRate;  // kolik satoshi za 1 CZK
const float feePercentage = 0.95;  // provize 5%

unsigned long currentBalanceSats = (currentBalance * satsPerCzk) * feePercentage;


WiFiClientSecure client;

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
  request += "Content-Type: application/json\r\r";
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

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Připojování k WiFi...");
  }
  
  Serial.println("Připojeno k WiFi!");

  String lnUrl = createWithdrawLink();
  Serial.println("LN URL: " + lnUrl);
}

void loop() {
  // zde můžete přidat kód pro periodickou aktualizaci nebo jinou funkcionalitu
}
