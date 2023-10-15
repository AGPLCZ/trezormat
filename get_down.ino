#include "WiFiS3.h"
#include "WiFiSSLClient.h" // Include pro HTTPS komunikaci
#include "IPAddress.h"

//arduino_secrets.h header file
#define SECRET_SSID "TP-LINK_9228"
#define SECRET_PASS "dekujizawifi"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;
WiFiSSLClient client; // Použití WiFiSSLClient pro HTTPS
char server[] = "dobrodruzi.cz"; 

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }
  printWifiStatus();
}

void loop() {
  httpRequest();
  delay(100000); // Pauza 10 sekund mezi každým požadavkem
}

void httpRequest() {
  client.stop();
  if (client.connect(server, 443)) { // Použití portu 443 pro HTTPS
    Serial.println("Connecting to server...");
    client.println("GET /mince.php HTTP/1.1");  
    client.println("Host: dobrodruzi.cz");
    client.println("Connection: close");
    client.println();

    delay(500); // Přidáno pro čekání na data od serveru

    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }

  } else {
    Serial.println("Connection failed");
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
