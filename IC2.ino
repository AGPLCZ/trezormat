#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <qrcode.h>

// Konfigurace displeje SSD1306 (změřte šířku a výšku vašeho displeje)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void setup() {
  // Inicializace displeje
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {  // Adresa I2C může být 0x3C nebo 0x3D
    for (;;);  // Zacyklujte, pokud inicializace selže
  }
  display.clearDisplay();  // Vyčistěte displej
}

void loop() {
  display.clearDisplay();  // Vyčistěte displej

  // Zobrazte text
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Vas text zde");
  display.display();  // Aktualizujte displej
  
  delay(2000);  // Počkejte 2 sekundy

  // Generujte a zobrazte QR kód
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, "Vas QR kod zde");

  display.clearDisplay();  // Vyčistěte displej
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(x*2, y*2, 2, 2, SSD1306_WHITE);  // Zvětšený QR kód o faktor 2
      }
    }
  }
  display.display();  // Aktualizujte displej

  delay(5000);  // Počkejte 5 sekund před další aktualizací
}
