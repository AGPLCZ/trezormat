#include <SPI.h>
#include <epd1in54_V2.h>  // Waveshare library for 1.54" e-paper display
#include <qrcode.h>

#define CS_PIN 10
#define DC_PIN 9
#define RESET_PIN 8
#define BUSY_PIN 7

Epd epd(CS_PIN, DC_PIN, RESET_PIN, BUSY_PIN);  // Create an instance of the Epd class

void setup() {
  SPI.begin();
  epd.Init();  // Initialize e-Paper display
}

void displayTextAndQR(String text) {
  epd.ClearFrame();  // Clear the frame

  epd.SetPartialWindow(0, 0, 200, 24);  // Set partial window for text
  epd.DisplayStringAt(10, 10, text.c_str(), &Font24, BLACK, WHITE);  // Display text
  epd.DisplayFrame();  // Display the frame

  // Generate QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text.c_str());

  epd.SetPartialWindow(10, 50, qrcode.size * 3, qrcode.size * 3);  // Set partial window for QR code
  epd.ClearFrame();  // Clear the frame
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        for (uint8_t dx = 0; dx < 3; dx++) {
          for (uint8_t dy = 0; dy < 3; dy++) {
            epd.SetPixel(x * 3 + dx + 10, y * 3 + dy + 50, BLACK);  // Enlarge QR code by a factor of 3
          }
        }
      }
    }
  }
  epd.DisplayFrame();  // Display the frame
}

void loop() {
  String text = String(millis() / 1000) + " sekund od startu";  // Convert milliseconds to seconds and append text
  displayTextAndQR(text);

  delay(10000);  // Wait for 10 seconds before the next update
}
