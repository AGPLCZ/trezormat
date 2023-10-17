#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include "IPAddress.h"
#include <ArduinoJson.h>

const char* ssid = "ag";
const char* password = "dekujizawifi";

const char* host = "wallet.paralelnipolis.cz";
const int httpsPort = 443;
const char* apiKey = "e***********************6";

WiFiSSLClient client;

String createWithdrawLink() {
  if (!client.connect(host, httpsPort)) {
    return "Připojení se nezdařilo!";
  }

  DynamicJsonDocument doc(1024);
  doc["title"] = "Můj automatický odkaz";
  doc["min_withdrawable"] = 20;
  doc["max_withdrawable"] = 20;
  doc["uses"] = 1;
  doc["wait_time"] = 60;
  doc["is_unique"] = true;
  doc["webhook_url"] = "https://dobrodruzi.cz/withdraw/index.php";
  doc["webhook_headers"] = "Content-Type: application/json";
  doc["webhook_body"] = "{\"amount\": 100}";
  doc["custom_url"] = "https://dobrodruzi.cz/withdraw/poslano.php";
  
  String json;
  serializeJson(doc, json);

  String request = "POST /withdraw/api/v1/links HTTP/1.1\r\n";
  request += "Host: " + String(host) + "\r\n";
  request += "X-Api-Key: " + String(apiKey) + "\r\n";
  request += "Content-Type: application/json\r\n";
  request += "Content-Length: " + String(json.length()) + "\r\n";
  request += "\r\n" + json;

  client.print(request);

  String response = "";
 while (client.connected()) {
  String line = client.readStringUntil('\n');
  if (line.length() <= 2) { // Konec hlaviček (prázdný řádek)
    break;
  }
  Serial.println(line); // Vypíše hlavičky
}
String responseBody = client.readString();
Serial.println(responseBody); // Vypíše tělo odpovědi


  return "Neočekávaná odpověď od serveru.";
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Připojování k WiFi...");
  }
  
  Serial.println("Připojeno k WiFi!");
  String fullResponse = client.readString();
Serial.println("Odpověď od serveru:");
Serial.println(fullResponse);

  String lnUrl = createWithdrawLink();
  Serial.println(lnUrl);
}

void loop() {
  // zde můžete přidat kód pro periodickou aktualizaci nebo jinou funkcionalitu
}
