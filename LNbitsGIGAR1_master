#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiSSLClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

const char* ssid = "ag";
const char* password = "dekujizawifi";
const char* host = "legend.lnbits.com";
const int httpsPort = 443;
const char* apiKey = "5f9940a3227d4df0904db50603e4d041";
WiFiSSLClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Připojování k WiFi...");
  }
  Serial.println("Připojeno k WiFi!");
}

void loop() {
  // zde můžete přidat kód pro periodickou aktualizaci nebo jinou funkcionalitu
  String lnUrl = createWithdrawLink();
  Serial.println("LN URL: " + lnUrl);
  delay(60000);  // Počkejte 1 minutu před dalším požadavkem
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
  doc["min_withdrawable"] = "1";
  doc["max_withdrawable"] = "1000000";
  doc["uses"] = 1;
  doc["wait_time"] = 60;
  doc["is_unique"] = true;
  
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
