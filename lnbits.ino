#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "VAŠE_SSID";
const char* password = "VAŠE_HESLO";

const char* host = "wallet.paralelnipolis.cz";
const int httpsPort = 443;
const char* apiKey = "e***********************6";

String createWithdrawLink() {
  WiFiClientSecure client;

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
    response = client.readStringUntil('\n');
    if (response.startsWith("HTTP/1.1 201")) {
      DynamicJsonDocument responseDoc(1024);
      deserializeJson(responseDoc, client);
      return responseDoc["lnurl"].as<String>();
    }
    // Zde můžete přidat další kontroly stavových kódů HTTP podle potřeby
  }

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
  
  String lnUrl = createWithdrawLink();
  Serial.println(lnUrl);
}

void loop() {
  // zde můžete přidat kód pro periodickou aktualizaci nebo jinou funkcionalitu
}
