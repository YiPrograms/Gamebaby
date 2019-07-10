#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <ESP8266WiFi.h>

// The display also uses hardware SPI, plus SD3, D4, D8
static uint8_t SD3 = 10;
#define TFT_CS SD3
#define TFT_DC D4
//Back Light LED
#define BL_LED D8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// width/height of the display when rotated horizontally
#define TFT_WIDTH  320
#define TFT_HEIGHT 240
#define SCOPE_WIDTH 320
#define SCOPE_HEIGHT 180
#define DATA_POINTS 40

//if you don't want to hear the speaker beep, define SPK_PIN to -1
#define SPK_PIN D1
//#define SPK_PIN -1

//  Variables
int PulseSensorPin = A0;        // Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
int val,threshold=512,maxvalue=1023,minvalue=0,deadzone=0;      
int values[DATA_POINTS];
int stableCount=0;
int pulseRate=70, prevPulseRate=70;
long int timeStamp[5], nowtime, prevtime;

// The SetUp Function:
void setup() {
   //Turn off WiFi to reduce the noise
   WiFi.disconnect();
   WiFi.mode(WIFI_OFF);

   pinMode(BL_LED, OUTPUT);
   digitalWrite(BL_LED, HIGH);
   Serial.begin(115200);         // Set's up Serial Communication at certain speed. 
   tft.begin();
   tft.setRotation(3);
   tft.fillScreen(ILI9341_BLACK);
   pinMode(SPK_PIN,OUTPUT);
}

int getY(int val) {
  return map(val,minvalue,maxvalue, SCOPE_HEIGHT-40,40);
}

int getValue(void) {
  int value = -1;
  value = analogRead(PulseSensorPin); 
  return value;
}

void pushValue(int value) {
  maxvalue = values[1];
  minvalue = values[1];
  for (int i=0; i<DATA_POINTS-1; i++)
  {
    values[i] = values[i+1];
    if (values[i]>maxvalue) {maxvalue= values[i];}
    else if (values[i]<minvalue) {minvalue= values[i];}
  }
  values[DATA_POINTS-1] = value;
  if (value>maxvalue) {maxvalue= value;}
  else if (value<minvalue) {minvalue= value;}
  threshold = (maxvalue-minvalue)*2/3+minvalue; //You can change /2 to *2/3
}

void drawLines(unsigned int color) {
  
  int x0 = 0;
  int y0 = getY(values[0]);
  for (int i=1; i<DATA_POINTS; i++) {
    int x1 = i*SCOPE_WIDTH/DATA_POINTS;
    int y1 = getY(values[i]);
    tft.drawLine(x0, y0, x1, y1, color);
    x0 = x1;
    y0 = y1;
  }
}

void drawGrid(void) {
  tft.drawRect(0, 0, SCOPE_WIDTH, SCOPE_HEIGHT, ILI9341_WHITE);
  tft.drawLine(0, SCOPE_HEIGHT/2, SCOPE_WIDTH, SCOPE_HEIGHT/2, ILI9341_RED);
}

void draw()
{
    val = getValue();
    drawLines(ILI9341_BLACK);
    pushValue(val);
    drawGrid();
    drawLines(ILI9341_YELLOW);
    drawHeart();
}

void drawHeart(void){
  tft.setCursor(80, 200); 
  tft.setTextColor(ILI9341_BLACK);    
  tft.setTextSize(4);
  tft.print("o");

  nowtime = millis();
  if ((values[DATA_POINTS-1] > (threshold+deadzone))&&(values[DATA_POINTS-2]<=(threshold-deadzone)))
  {
    timeStamp[stableCount] = nowtime;
    tft.setCursor(80, 200);
    tft.setTextColor(ILI9341_RED);    
    tft.setTextSize(4); 
    tft.print("o");
    tone(SPK_PIN,3520);
    pulseRate = (int)(60000.0 /(float)(nowtime-prevtime));
    if (pulseRate<220)
    {
      tft.fillRect(120,190,200,240,ILI9341_BLACK);
      tft.setCursor(120, 190);
      tft.setTextColor(ILI9341_CYAN);    
      tft.setTextSize(5); 
      tft.print(pulseRate);
    }
    prevtime = nowtime;      
  }
   
//  prevsize=size;
}

// The Main Loop Function



void loop() {

   draw();                                           // Assign this value to the "Signal" variable.

   Serial.println(val);                    // Send the Signal value to Serial Plotter.

   
   delay(20);
   
   noTone(SPK_PIN);
   
}
