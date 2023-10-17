#include <SPI.h>
#include <GxEPD2_BW.h>
#include <qrcode.h>

#define CS_PIN  10
#define DC_PIN  9
#define RESET_PIN  8
#define BUSY_PIN  7

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(CS_PIN, DC_PIN, RESET_PIN, BUSY_PIN));

void setup() {
  SPI.begin();
  display.init();
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
}

void displayTextAndQR(String text) {
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);

  display.firstPage();
  do {
    display.setCursor(10, 10);
    display.print(text);
  } while (display.nextPage());

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text.c_str());

  display.setPartialWindow(10, 50, qrcode.size * 3, qrcode.size * 3);
  display.firstPage();
  do {
    for (uint8_t y = 0; y < qrcode.size; y++) {
      for (uint8_t x = 0; x < qrcode.size; x++) {
        if (qrcode_getModule(&qrcode, x, y)) {
          display.fillRect(x * 3 + 10, y * 3 + 50, 3, 3, GxEPD_BLACK);
        }
      }
    }
  } while (display.nextPage());
}

void loop() {
  String text = String(millis() / 1000) + " sekund od startu";  // Convert milliseconds to seconds and append text
  displayTextAndQR(text);

  delay(10000);  // Wait for 10 seconds before the next update
}
