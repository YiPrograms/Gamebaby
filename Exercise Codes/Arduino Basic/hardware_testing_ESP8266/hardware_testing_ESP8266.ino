#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "XPT2046_Touchscreen.h"
#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

// Touchscreen
#define TOUCH_CS_PIN D3
#define TOUCH_IRQ_PIN D2
static uint8_t SD3 = 10;
#define TFT_CS SD3
#define TFT_DC D4
#define BL_LED D8

boolean pressed = false;
TS_Point touch_point;
XPT2046_Touchscreen ts(TOUCH_CS_PIN);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Calibration
// MINX=318 MINY=203
// MAXX=3951 MAXY=3859
int TS_MINX = 2000;
int TS_MINY = 2000;
int TS_MAXX = 2000;
int TS_MAXY = 2000;

// width/height of the display when rotated horizontally
#define TFT_WIDTH    320
#define TFT_HEIGHT   240
#define SCOPE_WIDTH  320
#define SCOPE_HEIGHT 180
#define DATA_POINTS   40

/*** I/O devices ***/
// tone  >turn-on: D1  >turn-off: -1
#define SPK_PIN D1
//#define SPK_PIN -1

// Heart beat
int PulseSensorPin = A0;

// DHT
#define DHTPIN D0      // what pin we're connected to
#define DHTTYPE DHT11  // DHT 11 
DHTesp dht;

// WIFI
/* Set these to your desired credentials. */
const char *ssid = "ESP8266_ap";
const char *password = "ntuee2019";

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
   connected to this access point to see it. */
void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected to this ESP8266!</h1>");
}

// Function for Calibration
void updateMinMax(TS_Point pt) {
  if (pt.x < TS_MINX) TS_MINX = pt.x;
  if (pt.y < TS_MINY) TS_MINY = pt.y;
  if (pt.x > TS_MAXX) TS_MAXX = pt.x;
  if (pt.y > TS_MAXY) TS_MAXY = pt.y;
}

void sensor_display() {
  tft.setTextColor(ILI9341_RED); 
  tft.setCursor(20, 140);
  tft.print("Heartbeat analogRead =");
  tft.fillRect(20, 160, 48, 16, ILI9341_BLACK);
  tft.setCursor(20, 160);
  tft.print(analogRead(PulseSensorPin));

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  // "nan == nan" returns false, therefore we can avoid displaying nan.
  if (humidity == humidity) {
    tft.setTextColor(ILI9341_CYAN); 
    tft.setCursor(20, 190);
    tft.print("humidity(\%)    =");
    tft.fillRect(220, 190, 48, 16, ILI9341_BLACK);
    tft.setCursor(220, 190);
    tft.print(humidity);

    tft.setCursor(20, 210);
    tft.print("temperature(C) =");
    tft.fillRect(220, 210, 48, 16, ILI9341_BLACK);
    tft.setCursor(220, 210);
    tft.print(temperature);

    Serial.print(dht.getStatusString());
    Serial.print("\t");
    Serial.print(humidity, 1);
    Serial.print("\t\t");
    Serial.print(temperature, 1);
    Serial.print("\t\t");
    Serial.print(dht.toFahrenheit(temperature), 1);
    Serial.print("\t\t");
    Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
    Serial.print("\t\t");
    Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
  }
  delay(20);
}

void setup() {
  pinMode(BL_LED, OUTPUT);
  pinMode(SPK_PIN,OUTPUT);
  digitalWrite(BL_LED, HIGH);
  dht.setup(DHTPIN, DHTesp::DHTTYPE); // Connect DHT sensor 

  Serial.begin(38400);
  tft.begin();
  tft.setRotation(3);
  tft.setTextSize(2);
  tft.fillScreen(ILI9341_BLACK);
  tft.drawRect(0, 0, 320, 240, ILI9341_YELLOW);
  tft.setTextColor(ILI9341_PINK);
  tft.setCursor(20, 20);
  tft.print("Touch the YELLOW border!");
  ts.begin();
  
  while (!Serial && (millis() <= 1000));
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  if (ts.touched()) {
    touch_point = ts.getPoint();
    updateMinMax(touch_point);
    tone(SPK_PIN, (touch_point.x + touch_point.y)/3 );

    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(20, 50);
    tft.print("pt_X = ");
    tft.fillRect(20 + 84, 50, 48, 16, ILI9341_BLACK);
    tft.print(touch_point.x);

    tft.setCursor(170, 50);
    tft.print("pt_Y = ");
    tft.fillRect(170 + 84, 50, 48, 16, ILI9341_BLACK);
    tft.print(touch_point.y);

    tft.setTextColor(ILI9341_GREEN);
    tft.setCursor(20, 80);
    tft.fillRect(20 + 84, 80, 48, 16, ILI9341_BLACK);
    tft.print("MINX = ");
    tft.print(TS_MINX);

    tft.setCursor(170, 80);
    tft.fillRect(170 + 84, 80, 48, 16, ILI9341_BLACK);
    tft.print("MAXX = ");
    tft.print(TS_MAXX);

    tft.setCursor(20, 100);
    tft.fillRect(20 + 84, 100, 48, 16, ILI9341_BLACK);
    tft.print("MINY = ");
    tft.print(TS_MINY);

    tft.setCursor(170, 100);
    tft.fillRect(170 + 84, 100, 48, 16, ILI9341_BLACK);
    tft.print("MAXY = ");
    tft.print(TS_MAXY);

    Serial.print(" x = ");
    Serial.print(touch_point.x);
    Serial.print(", y = ");
    Serial.println(touch_point.y);

    if (!pressed){
      pressed = true;
      tft.setTextColor(ILI9341_YELLOW);
      tft.setCursor(20, 20);
      tft.print("Touch the YELLOW border!");
    }
  }
  else if (pressed){
    pressed = false;
    noTone(SPK_PIN);
    tft.fillRect(20, 50, 290, 16, ILI9341_BLACK);
      
    tft.setTextColor(ILI9341_PINK);
    tft.setCursor(20, 20);
    tft.print("Touch the YELLOW border!");
  }

  server.handleClient();
  sensor_display();
}
