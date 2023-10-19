#include <GxEPD2_BW.h>
#include <qrcode.h>

// Definice pinů pro SPI komunikaci s adaptérem DESPI-C02
//SCK (DESPI-C02) -> Pin 13 (Arduino Uno)
//SDI (DESPI-C02) -> Pin 11 (Arduino Uno)
//CS -> Pin 10 (Arduino Uno)
//DC -> Pin 9 (Arduino Uno)
//RESET -> Pin 8 (Arduino Uno)
//BUSY -> Pin 7 (Arduino Uno)



#define CS_PIN    10
#define DC_PIN    9
#define RESET_PIN 8
#define BUSY_PIN  7

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(CS_PIN, DC_PIN, RESET_PIN, BUSY_PIN));

void setup()
{
  display.init();
  display.setRotation(1);
}

void loop()
{
  // Vytvoření QR kódu
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, "Vas QR kod zde");

  // Zobrazení QR kódu na e-Paper displeji
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    for (uint8_t y = 0; y < qrcode.size; y++)
    {
      for (uint8_t x = 0; x < qrcode.size; x++)
      {
        if (qrcode_getModule(&qrcode, x, y))
        {
          display.fillRect(x * 3, y * 3, 3, 3, GxEPD_BLACK);  // Zvětšení QR kódu o faktor 3
        }
      }
    }
  } while (display.nextPage());

  delay(10000);  // Zpoždění 10 sekund před dalším cyklem
}
