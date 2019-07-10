#include <SPI.h>
#include <Wire.h>  
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// For the Adafruit shield, these are the default.
#define TFT_DC D4
#define TFT_CS 10
#define BL_LED D8

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {
  Serial.begin(9600);
  Serial.println("ILI9341 Test!"); 
 
  tft.begin();
  pinMode(BL_LED,OUTPUT);
  digitalWrite(BL_LED,HIGH);

 }


void loop() {
  /*You can modify this part*/
  tft.setRotation(3);
  tft.fillScreen(ILI9341_YELLOW);
  delay(1000);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(50,50);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("Hello World!");
  delay(1000);
  /*You can modify this part*/
}
