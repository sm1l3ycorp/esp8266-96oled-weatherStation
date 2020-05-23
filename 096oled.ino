#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>  // http web access library
#include <ArduinoJson.h>        // JSON decoding library
#include <Wire.h>              // include wire library (for I2C devices such as the SSD1306 display)
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_SSD1306.h>  // include Adafruit SSD1306 OLED display driver
#include "timelib.h"
#include <ctime>
#include <string>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#include "icons.h"

SSD1306Wire display(0x3c, SDA, SCL);

const char* ssid  = "SSID";
const char* password = "PASSWORD";

// set location and API key
String Location = "Dallas,us";
String API_Key  = "APIKEY";

void setup(void)
{
  Serial.begin(9600);
  delay(1000);
  Wire.begin(4, 0);

  display.init();
  display.flipScreenVertically();

  display.clear();  // clear the display buffer
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawXbm(0, 5, 50, 50, clear_day_bits);
  display.drawXbm(64, 5, 50, 50, rain_bits);

  display.display();
  delay(1500);
  display.clear();

  WiFi.begin(ssid, password);
  display.display();
  while ( WiFi.status() != WL_CONNECTED )
  {
    delay(500);
    display.drawString(64, 22, "Connecting...");
    display.display();
  }
  display.clear();
  display.drawString(64, 22, "WIFI Connected!");
  display.display();
  delay(1000);
  display.clear();
  display.drawString(64, 22, "Fetching Weather...");
  display.display();
  delay(1000);
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)  //Check WiFi connection status
  {
    display.clear();
    display.drawString(64, 22, "Fetching Weather...");
    display.display();
    delay(1000);
    HTTPClient http;  //Declare an object of class HTTPClient

    http.begin("http://api.openweathermap.org/data/2.5/weather?q=" + Location + "&APPID=" + API_Key);  // !!
    int httpCode = http.GET();  // send the request

    if (httpCode > 0)
    {
      String payload = http.getString();

      DynamicJsonBuffer jsonBuffer(512);
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (!root.success()) {
        Serial.println(F("Parsing failed!"));
        return;
      }

      String location = root["name"];
      float temp = (float)(root["main"]["temp"]) - 273.15; // get temperature in °C
      float tempF = (temp * 9 / 5) + 32; // convert to °F
      int temperature = tempF;
      String conditions = root["weather"][0]["main"];
      String dateTime = root["dt"];

      time_t c;
      c = strtoul( dateTime.c_str(), NULL, 0 );
      Serial.println(ctime( &c ));
      String weekday = ctime( &c );
      weekday.remove(10, weekday.length());
      Serial.println(weekday);

      display.clear();
      display.drawString(25, 6, location);
      display.drawString(30, 50, String(temperature) + " ° " + conditions);
      display.drawString(25, 28, weekday);

      if (conditions.indexOf("Rain") == 0) {
        display.drawXbm(64, 2, 50, 50, rain_bits);
      } else if (conditions.indexOf("Clear") == 0) {
        display.drawXbm(64, 2, 50, 50, clear_day_bits);
      } else if (conditions.indexOf("Snow") == 0) {
        display.drawXbm(64, 2, 50, 50, snow_bits);
      } else if (conditions.indexOf("Clouds") == 0) {
        display.drawXbm(64, 2, 50, 50, partly_cloudy_day_bits);
      } else {
        display.drawXbm(64, 2, 50, 50, clear_day_bits);
      }
      display.display();
    } else {
      display.clear();
      display.drawString(64, 22, "API ERR0R!");
      display.display();
    }
    http.end();
  }
  delay(1800000); // 30 minutes
}
