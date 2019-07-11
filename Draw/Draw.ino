#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "XPT2046_Touchscreen.h"

// Touchscreen
#define TOUCH_CS_PIN D3
#define TOUCH_IRQ_PIN D2
static uint8_t SD3 = 10;
#define TFT_CS SD3
#define TFT_DC D4
#define BL_LED D8

bool RecordOn = false;
bool beentouched = false;


#define FRAME_X 210
#define FRAME_Y 180
#define FRAME_W 100
#define FRAME_H 50

#define REDBUTTON_X FRAME_X
#define REDBUTTON_Y FRAME_Y
#define REDBUTTON_W (FRAME_W/2)
#define REDBUTTON_H FRAME_H

#define GREENBUTTON_X (REDBUTTON_X + REDBUTTON_W)
#define GREENBUTTON_Y FRAME_Y
#define GREENBUTTON_W (FRAME_W/2)
#define GREENBUTTON_H FRAME_H

TS_Point touch_point;
XPT2046_Touchscreen ts(TOUCH_CS_PIN);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Calibration
// MINX=318 MINY=203
// MAXX=3951 MAXY=3859
int TS_MINX = 318;
int TS_MINY = 203;
int TS_MAXX = 3951;
int TS_MAXY = 3859;

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

void drawFrame()
{
  tft.drawRect(FRAME_X, FRAME_Y, FRAME_W, FRAME_H, ILI9341_BLACK);
}


void redBtn()
{ 
  tft.fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, ILI9341_RED);
  tft.fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, ILI9341_BLUE);
  drawFrame();
  tft.setCursor(GREENBUTTON_X + 6 , GREENBUTTON_Y + (GREENBUTTON_H/2));
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("ON");
  RecordOn = false;
}

void greenBtn()
{
  tft.fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, ILI9341_GREEN);
  tft.fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, ILI9341_BLUE);
  drawFrame();
  tft.setCursor(REDBUTTON_X + 6 , REDBUTTON_Y + (REDBUTTON_H/2));
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("OFF");
  RecordOn = true;
}


void setup() {
  pinMode(BL_LED, OUTPUT);
  pinMode(SPK_PIN,OUTPUT);
  digitalWrite(BL_LED, HIGH);
  tft.begin();
  if (!ts.begin()) { 
    //Serial.println("Unable to start touchscreen.");
  } 
  else { 
    //Serial.println("Touchscreen started."); 
  }
  tft.setRotation(3); 
  tft.fillScreen(ILI9341_BLACK);
    redBtn();
}

void loop() {
  if (ts.touched())
  { 
    if (!beentouched) {
        beentouched = true;
    // Retrieve a point  
    TS_Point p = ts.getPoint(); 

    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    int y = p.y;
    int x = p.x;

    if (RecordOn)
    {
      if((x > REDBUTTON_X) && (x < (REDBUTTON_X + REDBUTTON_W))) {
        if ((y > REDBUTTON_Y) && (y <= (REDBUTTON_Y + REDBUTTON_H))) {
          Serial.println("Red btn hit"); 
          redBtn();
        }
      }
    }
    else //Record is off (RecordOn == false)
    {
      if((x > GREENBUTTON_X) && (x < (GREENBUTTON_X + GREENBUTTON_W))) {
        if ((y > GREENBUTTON_Y) && (y <= (GREENBUTTON_Y + GREENBUTTON_H))) {
          Serial.println("Green btn hit"); 
          greenBtn();
        }
      }
    }

    Serial.println(RecordOn);

    //wait for release

    beentouched = false;
  }
    }
    TS_Point p = ts.getPoint(); 
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    int y = p.y;
    int x = p.x;
    if (RecordOn) {
        tft.drawLine(x, y, x, y, ILI9341_YELLOW);
        tft.drawLine(x-1, y, x+1, y, ILI9341_YELLOW);
        tft.drawLine(x, y-1, x, y+1, ILI9341_YELLOW);
        tft.drawLine(x-1, y-1, x+1, y-1, ILI9341_YELLOW);
        tft.drawLine(x-1, y-1, x-1, y+1, ILI9341_YELLOW);
    } else {
        tft.drawLine(x, y, x, y, ILI9341_BLACK);
        tft.drawLine(x-1, y, x+1, y, ILI9341_BLACK);
        tft.drawLine(x, y-1, x, y+1, ILI9341_BLACK);
        tft.drawLine(x-1, y-1, x+1, y-1, ILI9341_BLACK);
        tft.drawLine(x-1, y-1, x-1, y+1, ILI9341_BLACK);
    }
}
