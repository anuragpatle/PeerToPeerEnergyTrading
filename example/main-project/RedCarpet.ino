#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "pin_config.h"
#include "station_1.h"
#include "charging1.h"
#include <Wire.h>
#include "TSINLogo.h"
#include "Orbitron_Medium_20.h"
#include <ArduinoJson.h>
#include <FS.h>
#include <stdlib.h> // For random values

//-------------------------------------Display images

// JPEG decoder library
#include <JPEGDecoder.h>

/* The product now has two screens, and the initialization code needs a small change in the new version. The LCD_MODULE_CMD_1 is used to define the
 * switch macro. */
#define LCD_MODULE_CMD_1

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft); // Sprite class needs to be invoked
unsigned long targetTime = 0;        // Used for testing draw times

uint32_t const GREEN_1 = 0x3bbb2a;

#if defined(LCD_MODULE_CMD_1)
typedef struct
{
  uint8_t cmd;
  uint8_t data[14];
  uint8_t len;
} lcd_cmd_t;

lcd_cmd_t lcd_st7789v[] = {
    {0x11, {0}, 0 | 0x80},
    {0x3A, {0X05}, 1},
    {0xB2, {0X0B, 0X0B, 0X00, 0X33, 0X33}, 5},
    {0xB7, {0X75}, 1},
    {0xBB, {0X28}, 1},
    {0xC0, {0X2C}, 1},
    {0xC2, {0X01}, 1},
    {0xC3, {0X1F}, 1},
    {0xC6, {0X13}, 1},
    {0xD0, {0XA7}, 1},
    {0xD0, {0XA4, 0XA1}, 2},
    {0xD6, {0XA1}, 1},
    {0xE0, {0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32}, 14},
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14},
};
#endif

void setup()
{
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);        // Vertical screen
  tft.fillScreen(TFT_WHITE); // White background
  spr.setColorDepth(16);     // 16-bit color for smooth fonts

  Serial.println("Setup complete.");
}

void displayData()
{
  const char *t = "25";
  float airQualityAQI = random(45, 50);                // Hard-coded temperature value
  float h = random(40, 45);                            // Random humidity value between 4.0 and 5.9
  const char *lightValue = "Warm Glow";                // Hard-coded light value
  const char *musicValue = "Music: Don't let me down"; // Hard-coded music value

  // Draw static content once
  spr.createSprite(240, 320); // Create sprite of screen size
  spr.fillSprite(TFT_BLACK);  // White background

  // Print "Room Info" in fancy small font
  spr.setFreeFont(&FreeSerifBold12pt7b); // Use FreeSerifItalic9pt7b font for smaller text
  spr.setTextColor(TFT_GOLD);
  spr.setCursor(10, 20);
  spr.print("R O O M");

  // begin AQI
  spr.drawLine(0, 30, 255, 30, color24To565(0x931e1e));

  spr.setFreeFont(&FreeSerifBold9pt7b); // Use FreeSansBold24pt7b font for larger text
  spr.setTextColor(color24To565(0X2abb92));
  spr.setCursor(10, 50);
  spr.print("A I R");
  spr.setFreeFont(&FreeSerif12pt7b); // Use FreeSansBold24pt7b font for larger text

  spr.setTextColor(TFT_LIGHTGREY);
  spr.setCursor(10, 75);
  spr.print("" + String(airQualityAQI) + " AQI");
  // end AQI

  // Begin Temperature
  spr.drawLine(0, 90, 255, 90, color24To565(0x931e1e));
  spr.setFreeFont(&FreeSerifBold9pt7b); // Use FreeSansBold24pt7b font for larger text
  spr.setTextColor(color24To565(0X2abb92));
  spr.setCursor(10, 110);
  spr.print("T E M P.");
  spr.setFreeFont(&FreeSerif12pt7b); // Use FreeSansBold24pt7b font for larger text
  spr.setTextColor(TFT_LIGHTGREY);
  spr.setCursor(10, 135);
  spr.print("" + String(t) + " 'C");
  // End Temperature

  // Begin Humid
  spr.drawLine(0, 150, 255, 150, color24To565(0x931e1e));
  spr.setFreeFont(&FreeSerifBold9pt7b); // Use FreeSansBold24pt7b font for larger text
  spr.setTextColor(color24To565(0X2abb92));
  spr.setCursor(10, 170);
  spr.print("H U M I D");

  spr.setFreeFont(&FreeSerif12pt7b);
  spr.setTextColor(TFT_LIGHTGREY);
  spr.setCursor(10, 195);
  spr.print(String(h) + "%");
  // End Humid

  // Begin Light
  spr.drawLine(0, 205, 255, 205, color24To565(0x931e1e)); // Draw white horizontal line
  spr.setFreeFont(&FreeSerifBold9pt7b);                   // Use FreeSansBold24pt7b font for larger text
  spr.setTextColor(color24To565(0X2abb92));
  spr.setCursor(10, 225);
  spr.print("L I G H T");

  spr.setFreeFont(&FreeSerif12pt7b);
  spr.setTextColor(TFT_LIGHTGREY);
  spr.setCursor(10, 250);
  spr.print(String(lightValue));
  // End Light

  spr.pushSprite(0, 0); // Push sprite to screen
  spr.deleteSprite();   // Delete sprite to free memory

  // Begin Music

  // Horizontal scrolling of "Don't let me down"
  int scrollSpeed = 1; // Adjust scroll speed as needed
  int textWidth = tft.textWidth(musicValue);
  int xPos = 240; // Initial position off-screen

  while (true)
  {
    tft.fillRect(0, 280, 240, 40, TFT_BLACK); // Clear previous text area

    tft.setTextColor(color24To565(GREEN_1));
    tft.setFreeFont(&FreeSerif12pt7b);
    tft.setCursor(xPos, 280);
    tft.print(musicValue);

    xPos -= scrollSpeed;
    if (xPos < -textWidth)
    {
      xPos = 240; // Reset position once off-screen
    }

    delay(50); // Adjust delay for smoother scrolling
  }
}

void loop()
{
  displayData();
  delay(2000); // Update every 2 seconds
}

uint16_t color24To565(uint32_t color24)
{
  uint8_t r = (color24 >> 16) & 0xFF;
  uint8_t g = (color24 >> 8) & 0xFF;
  uint8_t b = color24 & 0xFF;
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}