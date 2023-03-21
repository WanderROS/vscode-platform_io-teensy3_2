#include <Arduino.h>
#include "../lib/TFT_ST7735/TFT_ST7735.h"
#include <SPI.h>
#include "../lib/TFT_ST7735/fonts/fluide_caps.h"
// Color definitions
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0

/*
Teensy3.x and Arduino's
You are using 4 wire SPI here, so:
 MOSI:  11//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 MISO:  12//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 SCK:   13//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 the rest of pin below:
 */
#define __CS 10
#define __DC 6
#define __RST 23
/*
Teensy 3.x can use: 2,6,10,15,20,21,22,23
Arduino's 8 bit: any
DUE: check arduino site
If you do not use reset, tie it to +3V3
*/

TFT_ST7735 tft = TFT_ST7735(__CS, __DC, __RST);

void setup()
{
  tft.begin();
  tft.setRotation(2);
  tft.setFont(&fluide_caps); // here we set the new font
  tft.setTextColor(BLUE);
  tft.print("HELLO,TEST");
  tft.setCursor(0, 40);
  tft.setFont(&internal); // this is the internal font, always loaded
  tft.print("hello World");
  // TFT 128*160
  tft.drawPixel(127, 159, RED);
  /*
  The external fonts can use all features like scaling, color, etc.
  */
}

void loop(void)
{
}