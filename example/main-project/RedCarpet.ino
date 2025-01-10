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
#include <stdlib.h>       // For random values
#include <WiFi.h>         // WiFi library for connecting to network
#include <PubSubClient.h> // MQTT library

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

// WiFi and MQTT configurations
// [BEGIN] Network Settings ---------------------------------------------------------------------------------------
const char *ssid = "A9";
const char *password = "999999999";
const char *mqtt_server = "raspberrypi.local";
#define LOG_TOPIC "client/logs"
// [END] Network Settings ---------------------------------------------------------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);

// Unique Client ID
String clientId = "TDisplayS3_Room-" + String(ESP.getEfuseMac());

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void publishLog(String message)
{
  String msgWithInfo = clientId + "_" + message;
  Serial.println(msgWithInfo); // Add this line to check messages
  client.publish(LOG_TOPIC, msgWithInfo.c_str());
  delay(100); // Add a delay to reduce flooding
}

void callback(char *topic, byte *message, unsigned int length)
{
  String logMessage = "Message arrived [";
  logMessage += topic;
  logMessage += "] ";

  String topic_str = String(topic);
  String topic_msg;
  for (int i = 0; i < length; i++)
  {
    logMessage += (char)message[i];
    topic_msg += (char)message[i];
  }

  publishLog(logMessage);

  // Handling Parking Light
  if (topic_str == "")
  {
  }
}
void reconnect()
{
  while (!client.connected())
  {
    String logMessage = "Attempting MQTT connection...";
    publishLog(logMessage);

    // Use the connect function with the clean session set to true
    if (client.connect(clientId.c_str(), nullptr, nullptr, nullptr, 0, true, nullptr))
    {
      publishLog("Connected");
      client.subscribe("screen/room", 0);
    }
    else
    {
      logMessage = "Failed, rc=" + String(client.state()) + " try again in 5 seconds";
      publishLog(logMessage);
      delay(5000);
    }
  }
}

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

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("Setup complete.");
}
void displayData()
{
  float t = random(23.1, 26.1);
  float airQualityAQI = random(45, 50);  // Random AQI value between 45 and 50
  float h = random(40.1, 45.1);          // Random humidity value between 40.1 and 45.1
  const char *lightValue = "Warm Glow";  // Hard-coded light value
  const char *musicValue = "soft music"; // Hard-coded music value

  // Draw static content once
  spr.createSprite(240, 320); // Create sprite of screen size
  spr.fillSprite(TFT_BLACK);  // Black background

  // Print "Room Info" in fancy small font
  spr.setFreeFont(&FreeSerifBold12pt7b); // Use FreeSerifBold12pt7b font for smaller text
  spr.setTextColor(TFT_GOLD);
  spr.setCursor(10, 20);
  spr.print("R O O M");

  // Begin AQI
  spr.drawLine(0, 30, 255, 30, color24To565(0x931e1e));

  spr.setFreeFont(&FreeSerifBold9pt7b); // Use FreeSerifBold9pt7b font for larger text
  spr.setTextColor(color24To565(0X2abb92));
  spr.setCursor(10, 50);
  spr.print("A I R");
  spr.setFreeFont(&FreeSerif12pt7b); // Use FreeSerif12pt7b font for larger text

  spr.setTextColor(TFT_LIGHTGREY);
  spr.setCursor(10, 75);
  spr.print(String(airQualityAQI) + " AQI");
  // End AQI

  // Begin Temperature
  spr.drawLine(0, 90, 255, 90, color24To565(0x931e1e));
  spr.setFreeFont(&FreeSerifBold9pt7b); // Use FreeSerifBold9pt7b font for larger text
  spr.setTextColor(color24To565(0X2abb92));
  spr.setCursor(10, 110);
  spr.print("T E M P.");
  spr.setFreeFont(&FreeSerif12pt7b); // Use FreeSerif12pt7b font for larger text
  spr.setTextColor(TFT_LIGHTGREY);
  spr.setCursor(10, 135);
  spr.print(String(t) + " 'C");
  // End Temperature

  // Begin Humid
  spr.drawLine(0, 150, 255, 150, color24To565(0x931e1e));
  spr.setFreeFont(&FreeSerifBold9pt7b); // Use FreeSerifBold9pt7b font for larger text
  spr.setTextColor(color24To565(0X2abb92));
  spr.setCursor(10, 170);
  spr.print("H U M I D");

  spr.setFreeFont(&FreeSerif12pt7b);
  spr.setTextColor(TFT_LIGHTGREY);
  spr.setCursor(10, 195);
  spr.print(String(h) + "%");
  // End Humid

  // Begin Light
  spr.drawLine(0, 205, 255, 205, color24To565(0x931e1e)); // Draw red horizontal line
  spr.setFreeFont(&FreeSerifBold9pt7b);                   // Use FreeSerifBold9pt7b font for larger text
  spr.setTextColor(color24To565(0X2abb92));
  spr.setCursor(10, 225);
  spr.print("L I G H T");

  spr.setFreeFont(&FreeSerif12pt7b);
  spr.setTextColor(TFT_LIGHTGREY);
  spr.setCursor(10, 250);
  spr.print(lightValue);
  // End Light

  // Begin Music
  spr.setFreeFont(&FreeSerif12pt7b); // Use FreeSerifBold9pt7b font for larger text
  spr.setTextColor(TFT_LIGHTGREY);
  spr.setCursor(10, 307);
  spr.print("SOFT MUSIC");
  // End Music

  spr.pushSprite(0, 0); // Push sprite to screen
  spr.deleteSprite();   // Delete sprite to free memory

  // Begin Wave Animation
  tft.fillRect(0, 270, 240, 15, TFT_BLACK); // Clear previous wave area

  for (int i = 0; i < 240; i += 5)
  {
    int waveHeight = random(5, 15);                                    // Random height for wave bars
    tft.drawLine(i, 275, i, 275 - waveHeight, color24To565(0x00FF00)); // Green bars
  }
  // End Wave Animation
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  displayData();
  delay(500); // Update every 100 ms for smoother animation
}

uint16_t color24To565(uint32_t color24)
{
  uint8_t r = (color24 >> 16) & 0xFF;
  uint8_t g = (color24 >> 8) & 0xFF;
  uint8_t b = color24 & 0xFF;
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
