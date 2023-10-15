#include "WiFiS3.h"
#include "WiFiSSLClient.h"

//#define SECRET_SSID "TP-LINK_9228"
#define SECRET_SSID "ag"
#define SECRET_PASS "dekujizawifi"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;
WiFiSSLClient client;

char server[] = "dobrodruzi.cz";
const int coin = 2;
volatile long totalInterrupts = 0;
unsigned long balance = 0;
unsigned long lastInterruptTime = 0;

void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(coin), coinInterrupt, RISING);
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop() {
  if (millis() - lastInterruptTime > 1 && totalInterrupts > 0) {
    balance += totalInterrupts;
    if (balance >= 75) {
      sendGetRequest(balance);
      balance = 0; // Reset balance after reaching the target
    }
    Serial.println("Zustatek: " + String(balance) + " Kc");
    totalInterrupts = 0;
  }
}
void sendGetRequest(unsigned long balance) {
  int attempts = 3;
  for (int i = 0; i < attempts; i++) {
    if (client.connect(server, 443)) {
      Serial.println("Connecting to server...");
      client.print("GET /mince.php?zaplaceno=1&kolik_vhozeno=");
      client.print(balance);
      client.println(" HTTP/1.1");
      client.println("Host: dobrodruzi.cz");
      client.println("Connection: close");
      client.println();
      break; // Připojení úspěšné, opustit cyklus
    } else {
      Serial.println("Connection to server failed");
      if (i < attempts - 1) { // Pokud to není poslední pokus
        Serial.println("Retrying...");
        delay(5000); // Pauza 5 sekund před dalším pokusem
      }
    }
  }
}


void coinInterrupt() {
  totalInterrupts++;
  lastInterruptTime = millis();
}
