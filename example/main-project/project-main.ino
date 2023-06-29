#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "img_logo.h"
#include "pin_config.h"
#include "station_1.h"

//-------------------------------------Display images

// JPEG decoder library
#include <JPEGDecoder.h>

// Return the minimum of two values a and b
#define minimum(a, b) (((a) < (b)) ? (a) : (b))

// Include the sketch header file that contains the image stored as an array of bytes
// More than one image array could be stored in each header file.
#include "jpeg1.h"
#include "jpeg2.h"
#include "jpeg3.h"
#include "jpeg4.h"

// Count how many times the image is drawn for test purposes
uint32_t icount = 0;
//----------------------------------------------------------------------------------------------------
//-------------------------------------Display images (end)

//---------------------------Analog Voltage Read for solar panel
int solarPanelPin = 1;
float energyGenerated = 10; // in KWh

//---------------------------Analog Voltage Read for solar panel

/* The product now has two screens, and the initialization code needs a small change in the new version. The LCD_MODULE_CMD_1 is used to define the
 * switch macro. */
#define LCD_MODULE_CMD_1

TFT_eSPI tft = TFT_eSPI();
#define WAIT 1000
unsigned long targetTime = 0; // Used for testing draw times

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
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  Serial.begin(115200);
  Serial.println("Hello T-Display-S3");
  tft.setTextSize(2); // Set the text size
  tft.setTextDatum(MC_DATUM);

  tft.begin();

#if defined(LCD_MODULE_CMD_1)
  for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++)
  {
    tft.writecommand(lcd_st7789v[i].cmd);
    for (int j = 0; j < lcd_st7789v[i].len & 0x7f; j++)
    {
      tft.writedata(lcd_st7789v[i].data[j]);
    }

    if (lcd_st7789v[i].len & 0x80)
    {
      delay(120);
    }
  }
#endif

  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, 320, 170, (uint16_t *)img_logo);
  delay(2000);

  ledcSetup(0, 2000, 8);
  ledcAttachPin(PIN_LCD_BL, 0);
  ledcWrite(0, 255);
}

void loop()
{

  tft.setRotation(0); // landscape
  tft.fillScreen(TFT_BLACK);

  // Draw a black rectangle on the right half of the screen
  tft.fillRect(0, tft.height() / 2 + 22, tft.width(), tft.height() / 2 + 22, TFT_LIGHTGREY);
  drawArrayJpeg(station_1, sizeof(station_1), 26.5, 186); // Draw a jpeg image stored in memory
                                                          // delay(5000000); // Delay for a short period of time
  tft.drawLine(0, 180, tft.width(), 180, TFT_LIGHTGREY);
  tft.drawLine(0, 179, tft.width(), 179, TFT_LIGHTGREY);
  tft.drawLine(0, 178, tft.width(), 178, TFT_LIGHTGREY);
  tft.drawLine(0, 72, tft.width(), 72, TFT_LIGHTGREY);

  targetTime = millis();

  while (true)
  {
    // Solar Panel
    float power = analogRead(solarPanelPin) * 2.5 / 4095; // Read voltage from ADC
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Solar Power", 72, 12, 1);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawFloat(power, 4, 50, 38, 2);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("kW", 142, 40, 2);

    // Station Energy Available
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Available", 60, 102, 1);
    tft.drawString("Energy", 42, 120, 1);

    // energy produced in one second
    energyGenerated = energyGenerated + (power / 3600); //  1 hour = 3600 second. Since, loop is running in each second
    tft.setTextColor(TFT_BROWN, TFT_BLACK);
    tft.drawFloat(energyGenerated, 4, 53, 145, 2);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("kWh", 145, 147, 2);

    // QR: Scan me to FUEL
    tft.setTextSize(2);
    tft.setTextColor(TFT_BACKLIGHT_ON, TFT_LIGHTGREY);
    tft.drawString("SCAN TO FUEL", 85, tft.height() - 10 , 1);

    delay(500);
  }

  delay(5000000); // Delay for a short period of time

  // First we test them with a background colour set
  // tft.setTextSize(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Available", 0, 48, 2);
  int xpos = 0;
  // xpos += tft.drawString("xyz{|}~", 0, 64, 2);
  tft.drawChar(127, xpos, 64, 2);
  delay(WAIT);

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BROWN, TFT_WHITE);

  tft.drawString("KWh", 0, 104, 4);
  delay(WAIT);

  tft.fillScreen(TFT_WHITE);
  tft.drawString("sometxt", 0, 0, 4);
  xpos = 0;
  // xpos += tft.drawString("{|}~", 0, 78, 4);
  tft.drawChar(127, xpos, 78, 4);
  delay(WAIT);

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  tft.drawString("012345", 0, 0, 6);
  tft.drawString("6789", 0, 40, 6);
  tft.drawString("apm-:.", 0, 80, 6);
  delay(WAIT);

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  tft.drawString("0123", 0, 0, 7);
  tft.drawString("4567", 0, 60, 7);
  delay(WAIT);

  tft.fillScreen(TFT_WHITE);
  tft.drawString("890:.", 0, 0, 7);
  tft.drawString("", 0, 60, 7);
  delay(WAIT);

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  tft.drawString("01", 0, 0, 8);
  delay(WAIT);

  tft.drawString("23", 0, 0, 8);
  delay(WAIT);

  tft.drawString("45", 0, 0, 8);
  delay(WAIT);

  tft.drawString("67", 0, 0, 8);
  delay(WAIT);

  tft.drawString("89", 0, 0, 8);
  delay(WAIT);

  tft.drawString("0:.", 0, 0, 8);
  delay(WAIT);

  tft.setTextColor(TFT_MAGENTA);
  tft.drawNumber(millis() - targetTime, 0, 100, 4);
  delay(4000);

  // Now test them with transparent background
  targetTime = millis();

  // tft.setTextSize(1);
  // tft.fillScreen(TFT_WHITE);
  // tft.setTextColor(TFT_BLACK, TFT_WHITE);

  // tft.drawString(" !\"#$%&'()*+,-./0123456", 0, 0, 2);
  // tft.drawString("789:;<=>?@ABCDEFGHIJKL", 0, 16, 2);
  // tft.drawString("MNOPQRSTUVWXYZ[\\]^_`", 0, 32, 2);
  // tft.drawString("abcdefghijklmnopqrstuvw", 0, 48, 2);
  // xpos = 0;
  // xpos += tft.drawString("xyz{|}~", 0, 64, 2);
  // tft.drawChar(127, xpos, 64, 2);
  // delay(WAIT);

  // tft.fillScreen(TFT_WHITE);
  // tft.setTextColor(TFT_BLACK, TFT_WHITE);

  // tft.drawString(" !\"#$%&'()*+,-.", 0, 0, 4);
  // tft.drawString("/0123456789:;", 0, 26, 4);
  // tft.drawString("<=>?@ABCDE", 0, 52, 4);
  // tft.drawString("FGHIJKLMNO", 0, 78, 4);
  // tft.drawString("PQRSTUVWX", 0, 104, 4);

  // delay(WAIT);
  tft.fillScreen(TFT_WHITE);
  tft.drawString("pqrstuvwxyz", 0, 52, 4);
  xpos = 0;
  xpos += tft.drawString("{|}~", 0, 78, 4);
  tft.drawChar(127, xpos, 78, 4);
  delay(WAIT);

  // tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLUE);

  tft.drawString("apm-:. 99 ", 0, 80, 6);
  delay(WAIT);

  // tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  tft.drawString("0123", 0, 0, 7);
  delay(WAIT);

  // tft.fillScreen(TFT_WHITE);
  tft.drawString("890:.", 0, 0, 7);
  tft.drawString("", 0, 60, 7);
  delay(WAIT);

  // tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  tft.drawString("0123", 0, 0, 8);
  delay(WAIT);

  // tft.fillScreen(TFT_WHITE);
  tft.drawString("4567", 0, 0, 8);
  delay(WAIT);

  // tft.fillScreen(TFT_WHITE);
  tft.drawString("890:.", 0, 0, 8);
  delay(WAIT);

  tft.setTextColor(TFT_MAGENTA, TFT_WHITE);

  tft.drawNumber(millis() - targetTime, 0, 100, 4);
  delay(4000);
}

// ####################################################################################################
//  Draw a JPEG on the TFT pulled from a program memory array
// ####################################################################################################
void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos)
{

  int x = xpos;
  int y = ypos;

  JpegDec.decodeArray(arrayname, array_size);

  jpegInfo(); // Print information from the JPEG file (could comment this line out)

  renderJPEG(x, y);

  Serial.println("#########################");
}

// ####################################################################################################
//  Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
// ####################################################################################################
//  This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
//  fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void renderJPEG(int xpos, int ypos)
{

  // retrieve infomration about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.readSwappedBytes())
  {

    // save a pointer to the image block
    pImg = JpegDec.pImage;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos; // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x)
      win_w = mcu_w;
    else
      win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y)
      win_h = mcu_h;
    else
      win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // draw image MCU block only if it will fit on the screen
    if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
    {
      tft.pushRect(mcu_x, mcu_y, win_w, win_h, pImg);
    }
    else if ((mcu_y + win_h) >= tft.height())
      JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }

  // calculate how long it took to draw the image
  drawTime = millis() - drawTime;

  // print the results to the serial port
  Serial.print(F("Total render time was    : "));
  Serial.print(drawTime);
  Serial.println(F(" ms"));
  Serial.println(F(""));
}

// ####################################################################################################
//  Print image information to the serial port (optional)
// ####################################################################################################
void jpegInfo()
{
  Serial.println(F("==============="));
  Serial.println(F("JPEG image info"));
  Serial.println(F("==============="));
  Serial.print(F("Width      :"));
  Serial.println(JpegDec.width);
  Serial.print(F("Height     :"));
  Serial.println(JpegDec.height);
  Serial.print(F("Components :"));
  Serial.println(JpegDec.comps);
  Serial.print(F("MCU / row  :"));
  Serial.println(JpegDec.MCUSPerRow);
  Serial.print(F("MCU / col  :"));
  Serial.println(JpegDec.MCUSPerCol);
  Serial.print(F("Scan type  :"));
  Serial.println(JpegDec.scanType);
  Serial.print(F("MCU width  :"));
  Serial.println(JpegDec.MCUWidth);
  Serial.print(F("MCU height :"));
  Serial.println(JpegDec.MCUHeight);
  Serial.println(F("==============="));
}

// ####################################################################################################
//  Show the execution time (optional)
// ####################################################################################################
//  WARNING: for UNO/AVR legacy reasons printing text to the screen with the Mega might not work for
//  sketch sizes greater than ~70KBytes because 16 bit address pointers are used in some libraries.

// The Due will work fine with the HX8357_Due library.

void showTime(uint32_t msTime)
{
  // tft.setCursor(0, 0);
  // tft.setTextFont(1);
  // tft.setTextSize(2);
  // tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // tft.print(F(" JPEG drawn in "));
  // tft.print(msTime);
  // tft.println(F(" ms "));
  Serial.print(F(" JPEG drawn in "));
  Serial.print(msTime);
  Serial.println(F(" ms "));
}
