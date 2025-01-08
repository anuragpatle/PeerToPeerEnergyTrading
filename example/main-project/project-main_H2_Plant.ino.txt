#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "pin_config.h"
#include "station_3.h"
#include "charging1.h"
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "TSINLogo.h"
#include "Orbitron_Medium_20.h"
#include <ArduinoJson.h>
#include "DHT.h"

//-------------------------------------Display images

// JPEG decoder library
#include <JPEGDecoder.h>

// Return the minimum of two values a and b
#define minimum(a, b) (((a) < (b)) ? (a) : (b))

#define WATER_LEVEL_SIGNAL_PIN 1
#define H2_PPM_SIGNAL_PIN 2
#define DHTPIN 17 // humidity and temperature sensor pin
#define SOIL_MOISTURE_SENSOR 4
#define DHTTYPE DHT11

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

const char *ssid = "A9";
const char *password = "999999999";
// const char *ssid = "CWLANGuest";
// const char *password = "welcomeguest";

int waterLevelSensorValue = 0; // variable to store the sensor value
int h2GasSensorValue = 0;      // variable to store the sensor value

// Your Domain name with URL path or IP address with path
// const char *serverName = "20.96.116.65:80/api/v1/updateStationState/2";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
// unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
// unsigned long timerDelay = 5000;

DHT dht(DHTPIN, DHTTYPE); // constructor to declare our sensor

void setup()
{

  Serial.begin(115200);

  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  dht.begin();

  Serial.println("Hello T-Display-S3");
  tft.begin();

  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.setTextSize(1); // Set the text size
  tft.setTextDatum(MC_DATUM);
  tft.setFreeFont(&Orbitron_Medium_20);

  // tft.pushImage(0, 0, 320, 170, (uint16_t *)img_logo);
  // delay(2000);

  // ledcSetup(0, 2000, 8);
  // ledcAttachPin(PIN_LCD_BL, 0);
  // ledcWrite(0, 255);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Water Level", 80, 20, 1);

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("m", 150, 80);

  tft.drawLine(0, 100, tft.width(), 100, TFT_SKYBLUE);

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("H2", 80, 110, 1);

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("PPM", 180, 140);
  tft.drawLine(0, 200, tft.width(), 200, TFT_SKYBLUE);

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
    Serial.println("Connecting WiFi " + (String)ssid);
    WiFi.begin(ssid, password);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Connecting", 70, 100, 1);
    tft.drawString("to WiFi..", 70, 130, 1);
    tft.drawString((String)ssid, 50, 160, 1);
    delay(15000);
    tft.fillScreen(TFT_BLACK); // Clear the screen
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{

  // testChargingUI();
  // delay(99999999);

  // Connect to Wi-Fi network
  // connectToWifi();

  delay(10);                                                  // wait 10 milliseconds
  waterLevelSensorValue = analogRead(WATER_LEVEL_SIGNAL_PIN); // read the analog value from sensor

  Serial.print("The water sensor value: ");
  Serial.println(waterLevelSensorValue);

  // tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
  // tft.setTextSize(2);
  tft.drawNumber(waterLevelSensorValue, 25, 70, 6);

  int h2SensorValue = analogRead(H2_PPM_SIGNAL_PIN); /*Analog value read function*/
  Serial.print("Gas Sensor: ");
  Serial.print(h2SensorValue); /*Read value printed*/
  Serial.print("\t");
  Serial.print("\t");
  if (h2SensorValue > 1800)
  { /*if condition with threshold 1800*/
    Serial.println("Gas");
    // digitalWrite(LED, HIGH); /*LED set HIGH if Gas detected */
  }
  else
  {
    Serial.println("No Gas");
    // digitalWrite(LED, LOW); /*LED set LOW if NO Gas detected */
  }

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  // tft.setTextSize(2);
  tft.drawNumber(h2SensorValue, 54, 140, 4);

  delay(1000);
}
