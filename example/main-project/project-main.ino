#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "pin_config.h"
#include "station_1.h"
#include "charging1.h"
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "TSINLogo.h"
#include "Orbitron_Medium_20.h"

//-------------------------------------Display images

// JPEG decoder library
#include <JPEGDecoder.h>

// Return the minimum of two values a and b
#define minimum(a, b) (((a) < (b)) ? (a) : (b))

//---------------------------Analog Voltage Read for solar panel
char *HOME_SCREEN = "READINGS_AND_QR_SCREEN";
char *WHILE_CHARGING_SCREEN = "WHILE_CHARGING_SCREEN";

int solarPanelPin = 1;
float energyGenerated = 10.0000; // in KWh
char *present_screen_mode = HOME_SCREEN;
char *lastScreenMode = WHILE_CHARGING_SCREEN;
bool toggleScreenMode = true;
bool isVehicleCharging = false;
char *vehicleChargingState = "FUELING";
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

const char *ssid = "Neo6";
const char *password = "999999999";
// const char *ssid = "CWLANGuest";
// const char *password = "welcomeguest";

// Your Domain name with URL path or IP address with path
const char *serverName = "20.96.116.65:80/api/v1/updateStationState/2";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
// unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
// unsigned long timerDelay = 5000;

void setup()
{

  Serial.begin(115200);

  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  Serial.println("Hello T-Display-S3");
  tft.begin();

  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.setTextSize(1); // Set the text size
  tft.setTextDatum(MC_DATUM);
  // tft.setFreeFont(&Orbitron_Medium_20);

  // tft.pushImage(0, 0, 320, 170, (uint16_t *)img_logo);
  // delay(2000);

  // ledcSetup(0, 2000, 8);
  // ledcAttachPin(PIN_LCD_BL, 0);
  // ledcWrite(0, 255);
  tft.fillScreen(TFT_BLACK);

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
}

void connectToWifi()
{

  Serial.println(WiFi.status()); // should be equal to 3
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Trying to connect to WiFi: " + (String)ssid);
    WiFi.begin(ssid, password);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Connecting to WiFi..", 60, 100, 1);
    tft.drawString((String)ssid, 50, 130, 1);
    delay(20000);
    tft.fillScreen(TFT_BLACK); // Clear the screen
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
int count = 0;
void loop()
{

  // ++count;

  // if ((count % 7) == 0)
  // {
  //   isVehicleCharging = true;
  // }
  // else
  // {
  //   isVehicleCharging = false;
  // }

  // Connect to Wi-Fi network
  // connectToWifi();

  tft.fillScreen(TFT_BLACK);
  // drawArrayJpeg(tsinlogo, sizeof(tsinlogo), 0, tft.height() - 25); // Draw a jpeg image stored in memory
  tft.pushImage(30, tft.height() - 25, 105, 25, TSINLogo);

  while (true)
  {
    drawArrayJpeg(charging1, sizeof(charging1), 40, 70); // Draw a jpeg image stored in memory
    delay(1000);
    tft.fillRect(0, 26, tft.width(), tft.height() - 60, TFT_BLACK);
    // tft.setTextSize(2);
    tft.setFreeFont(&Orbitron_Medium_20);
    tft.setCursor(6, 82);
    tft.println("town");
    // tft.drawString("Time: ", 10, 20, 2);
    // tft.setTextColor(TFT_BROWN, TFT_BLACK);
    // tft.setTextColor(0xffff);
    // tft.drawString("44", 50, 20, 2);

    delay(500);
  }

  if (isVehicleCharging)
  {
    present_screen_mode = WHILE_CHARGING_SCREEN;
  }
  else
  {
    present_screen_mode = HOME_SCREEN;
  }

  if (strcmp(present_screen_mode, HOME_SCREEN) == 0)
  {

    if (strcmp(lastScreenMode, WHILE_CHARGING_SCREEN) == 0)
    {

      tft.fillScreen(TFT_BLACK);

      toggleScreenMode = false;
      // Draw a black rectangle on the right half of the screen
      tft.fillRect(0, tft.height() / 2 + 22, tft.width(), tft.height() / 2 + 22, TFT_LIGHTGREY);
      drawArrayJpeg(station_1, sizeof(station_1), 26.5, 186); // Draw a jpeg image stored in memory
                                                              // delay(5000000); // Delay for a short period of time
      // tft.drawLine(0, 180, tft.width(), 180, TFT_SKYBLUE);
      // tft.drawLine(0, 179, tft.width(), 179, TFT_SKYBLUE);
      // tft.drawLine(0, 178, tft.width(), 178, TFT_SKYBLUE);
      tft.drawLine(0, 72, tft.width(), 72, TFT_SKYBLUE);
      lastScreenMode = HOME_SCREEN;
    }

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
    // tft.setTextSize(15);
    tft.setTextColor(TFT_BACKLIGHT_ON, TFT_LIGHTGREY);
    tft.drawString("SCAN TO FUEL", 85, tft.height() - 10, 1);

    updateEnergyAvailableAndRateOfEnergyGeneration(energyGenerated, power);
  }

  if (strcmp(present_screen_mode, WHILE_CHARGING_SCREEN) == 0 && strcmp(lastScreenMode, HOME_SCREEN) == 0)
  {

    if (strcmp(vehicleChargingState, "NOT_INITIATED"))
    {
      tft.fillScreen(TFT_LIGHTGREY);
      tft.setTextSize(1);
      tft.setTextColor(TFT_BACKLIGHT_ON, TFT_LIGHTGREY);
      tft.drawString("Initiating charging..", 85, tft.height() - 10, 1);
    }

    delay(4000);

    if (strcmp(vehicleChargingState, "FUELING"))
    {
      tft.fillScreen(TFT_BLACK);
      drawArrayJpeg(charging1, sizeof(charging1), 0, 60); // Draw a jpeg image stored in memory
      while (strcmp(vehicleChargingState, "FUELING"))
      {
        tft.setTextSize(1);
        tft.setTextColor(TFT_BACKLIGHT_ON, TFT_WHITE);
        tft.drawString("CHARGING.  ", 85, tft.height() - 10, 1);
        delay(100);
        tft.drawString("CHARGING.. ", 85, tft.height() - 10, 1);
        delay(100);
        tft.drawString("CHARGING...", 85, tft.height() - 10, 1);
        delay(100);
      }
    }

    if (strcmp(vehicleChargingState, "FUELING_COMPLETE_AND_PENDING_FOR_PAYMENT"))
    {
      tft.fillScreen(TFT_LIGHTGREY);
      tft.setTextSize(2);
      tft.setTextColor(TFT_BACKLIGHT_ON, TFT_LIGHTGREY);
      tft.drawString("Waiting for the payment", 85, tft.height() - 10, 1);
    }
    delay(4000);

    if (strcmp(vehicleChargingState, "COMPLETE"))
    {
      tft.fillScreen(TFT_LIGHTGREY);
      tft.setTextSize(2);
      tft.setTextColor(TFT_BACKLIGHT_ON, TFT_LIGHTGREY);
      tft.drawString("Charging complete..", 85, tft.height() - 10, 1);
    }
    delay(4000);

    lastScreenMode = WHILE_CHARGING_SCREEN;
  }

  delay(500);
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

// void showTime(uint32_t msTime)
// {
//   // tft.setCursor(0, 0);
//   // tft.setTextFont(1);
//   // tft.setTextSize(2);
//   // tft.setTextColor(TFT_WHITE, TFT_BLACK);
//   // tft.print(F(" JPEG drawn in "));
//   // tft.print(msTime);
//   // tft.println(F(" ms "));
//   Serial.print(F(" JPEG drawn in "));
//   Serial.print(msTime);
//   Serial.println(F(" ms "));
// }

void updateEnergyAvailableAndRateOfEnergyGeneration(float capacity, float rateOfGeneration)
{

  char strCapacity[10];
  dtostrf(capacity, 6, 6, strCapacity);

  char strRateOfGeneration[10];
  dtostrf(rateOfGeneration, 6, 6, strRateOfGeneration);

  HTTPClient http;
  String url = "http://20.96.116.65:80/api/v1/updateStationState/3/" + String(strCapacity) + "/" + String(strRateOfGeneration) + "";
  // String updateEnergy = "http://20.96.116.65:80/api/v1/updateStationEnergy/2";
  // String getStationsStateURL = "http://20.96.116.65:80:80/api/v1/getStationsState/";

  // String body = "PUT sent from ESP32";

  // Create a JSON object
  //  energy.deviceId = req.body.deviceId,
  //    energy.stationType= req.body.stationtype,
  //    energy.capacity = req.body.capacity,
  //    energy.rateOfEnergyGeneration = req.body.rateOfEnergyGeneration,
  //    energy.qrCode = req.body.qrcode,
  //    energy.charging = req.body.charging;

  String body = "[{\"rateOfEnergyGeneration\": \"99\",\"capacity\":\"99\"}]";
  // String jsonPayload = "{ \"deviceId\":\"2\", "\stationtype\":"\\"stationId\": \"4\", \"rateOfEnergyGeneration\": \"55\", \"capacity\":\"40511\" }";

  // String energyPayload = 66
  http.addHeader("Content-Type", "application/json");
  // http.addHeader("Content-Type", "text/plain");
  // http.addHeader("Content-Length", "65");
  // http.addHeader("Accept", "*/*");
  http.begin(url);
  // http.PUT(httpRequestBody);
  // Set the content type header to indicate JSON data

  int httpResponseCode = http.PUT("{ \"myKey\": \"99\"}");
  // int httpResponseCode = http.GET();
  if (httpResponseCode > 0)
  {
    Serial.println("HTTP Response Code: " + String(httpResponseCode));
    Serial.println("HTTP Response : " + String(http.getString()));
  }
  else
  {
    Serial.println("Error on HTTP request" + String(httpResponseCode));
  }
  http.end();
  delay(500);
}
