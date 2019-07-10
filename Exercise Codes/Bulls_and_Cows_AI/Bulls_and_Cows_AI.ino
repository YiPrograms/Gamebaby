#include <Adafruit_GFX.h>  // Core graphics library
#include <SPI.h>
#include <Wire.h>  // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <string.h>
#include "buttonCoordinate.h"

// Calibration data for the raw touch data to the screen coordinates
#define TS_MINX 361
#define TS_MINY 263
#define TS_MAXX 3922
#define TS_MAXY 3853


#define TOUCH_CS_PIN D3
#define TOUCH_IRQ_PIN D2

XPT2046_Touchscreen ts(TOUCH_CS_PIN, TOUCH_IRQ_PIN);


// The display also uses hardware SPI, plus #9 & #10
static uint8_t SD3 = 10;
#define TFT_CS SD3
#define TFT_DC D4
#define BL_LED D8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

/*
 * ILI9341_BLACK   0x0000
 * ILI9341_BLUE    0x001F
 * ILI9341_RED     0xF800
 * ILI9341_GREEN   0x07E0
 * ILI9341_CYAN    0x07FF
 * ILI9341_MAGENTA 0xF81F
 * ILI9341_YELLOW  0xFFE0
 * ILI9341_WHITE   0xFFFF
 */

ButtonCoordinate startButton, quitButton, reverseButton, reguessButton;
ButtonCoordinate numButtons[10];

enum GameState{menuMode, reverseMode, endMode};
GameState gameState;

struct GameData {
    char R[6];
    char guess[6];
    bool histR[10];
    bool histG[10];
    int A;
    int B;
    int count;
} gameData;

int posss[30250];
bool poss[30250];

struct AIData {
    bool possible[10000];
    int guess;
    int A;
    int B;
    int count;
    int remain;
} AIData;

void drawStartScreen() {
    tft.fillScreen(ILI9341_BLACK);

    //Draw white frame
    tft.drawRect(0,0,319,240,ILI9341_WHITE);

    tft.setCursor(80,30);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(4);
    tft.print("Arduino");

    tft.setCursor(18,90);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.print("Bulls & Cows");

    tft.setCursor(110,130);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.print("By AI");

    startButton.fillAndDraw(tft, ILI9341_RED, ILI9341_WHITE);
    tft.setCursor(118,190);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.print("Start");
}

void drawVerticalLine(int x) {
    for(int i=0;i<5;i++) {
        tft.drawLine(x+i,112,x+i,240,ILI9341_WHITE);
    }
}

void drawHorizontalLine(int y) {
    for(int i=0;i<5;i++) {
        tft.drawLine(0,y+i,320,y+i,ILI9341_WHITE);
    }
}

void printNumber(int index) {
    // char* num_str[10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    int offset = 16;
    // int x = offset + (index % 5) * 64;
    // int y = offset + 112 + (index / 5) * 64;
    int x = offset + numButtons[index].x;
    int y = offset + numButtons[index].y;
    tft.setCursor(x, y);
    tft.print(index);
}

// AI mode: AI guess your number

void drawAIScreen() {
    tft.fillScreen(ILI9341_BLACK);
    tft.drawRect(0,0,319,240,ILI9341_WHITE);
    for(int i = 0; i < 4; ++i) {
        drawVerticalLine(64 * (i+1) - 2);
    }
    for(int i = 0; i < 4; ++i) {
        drawHorizontalLine(112 + 64 * i - 2);
    }
    for(int i = 0; i < 5; ++i) {
        printNumber(i);
    }
    tft.setCursor(192 + 8, 0 + 8);
    tft.print("Back");
    // draw guess
    tft.setCursor(8, 8);
    if(AIData.remain > 0) {
        if(AIData.guess < 1000) tft.print("0");
        tft.print(AIData.guess);
    }
    else {
        tft.print("Impossible!");
    }
    // draw result
    if(AIData.A == 5) {
        tft.setCursor(8, 37 + 8);
        tft.print("Game Over!");
    }
    else {
        tft.setCursor(160 + 8, 37 + 8);
        tft.print("ReGuess");
        tft.setCursor(8, 37 + 8);
        tft.print("A: ");
        if(AIData.A >= 0) tft.print(AIData.A);
        tft.setCursor(8, 74 + 8);
        tft.print("B: ");
        if(AIData.B >= 0) tft.print(AIData.B);
    }
    tft.setCursor(128 + 8, 74 + 8);
    tft.print("count: ");
    tft.print(AIData.count);
}

void AIMove() {
    // AI guess a number
    resetAIGuess();
    
    /* TODO 01
     *  your code here
     */
    int index = random(0, AIData.remain);
    int counter = 0;
    for(int i= 0; i< 100000; ++i) {
        if(AIData.possible[i]) 
        ++counter;
        if(counter > index) {
            AIData.guess= i;
            break;
         }
     }
    
    ++AIData.count;
}

void PlayerMove() {
    // player enter result
    int count = 0;
    while(count < 2) {
        if(ts.touched()) {
            TS_Point p0 = ts.getPoint();  //Get touch point
            TS_Point p;
            p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
            p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);
            Serial.print("X = "); Serial.print(p.x);
            Serial.print("\tY = "); Serial.print(p.y);
            Serial.print("\n");
            if(quitButton.isPressed(p.x, p.y)) {
                gameState = endMode;
                break;
            }
            if(reguessButton.isPressed(p.x, p.y)) {
                AIMove();
                --AIData.count;
                drawAIScreen();
            }
            for(int i = 0; i < 5; ++i) {
                if(numButtons[i].isPressed(p.x, p.y)) {
                    if(count == 0) AIData.A = i;
                    else if(count == 1) AIData.B = i;
                    ++count;
                    drawAIScreen();
                }
            }
            if(AIData.A == 5) {
                gameState = endMode;
                delay(2000);
                break;
            }
        }
        delay(50); // prevent wdt reset and continuous input
    }
    ++gameData.count;
}

void AIFilter() {
    Serial.print("filter\n");
    // AI filters out possibilities by player result
    if(gameState != reverseMode) return;

    /* TODO 02
     *  your code here
     */
    bool histG[10]={0};
    int guess[5]={0};
    int temp=AIData.guess;
    for (int i=0; i<5; i++) {
        guess[i]=temp%10;
        histG[temp%10]=true;
        temp/=10;
    }
    for (int i=0; i<100000; i++) {
        if(AIData.possible[i]) {
            bool histR[10]={0};
            int target[5]={0};
            int temp = i;
            int A = 0;
            int B = 0;
            for(int j=0; j<5; j++) {
                target[j]=temp%10;
                histR[temp%10]=true;
                temp/=10;
            }
            for(int j=0; j<5; j++) {
               if(guess[j]==target[j]) A++;
            }
            for(int j=0; j<10; j++) {
                if(histG[j] && histR[j]) B++;
            }
            B -= A;
            if((A!=AIData.A) || (B!=AIData.B)) {
                // remove this number from possible list
                AIData.possible[i]=false;
                --AIData.remain;
            }
        }
    }
        

    Serial.print("remain: ");
    Serial.print(AIData.remain);
    Serial.print("\n");
    if(AIData.remain == 0) {
        drawAIScreen();
        gameState = endMode;
        delay(2000);
    }
    else {
        for(int i = 0; i < 100000; ++i) {
            if(AIData.possible[i]) {
                Serial.print("first possible: ");
                Serial.print(i);
                Serial.print("\n");
                break;
            }
        }
    }
}

void resetAIGuess() {
    AIData.A = -1;
    AIData.B = -1;
    AIData.guess = 0;
}

void resetAIData() {
    Serial.print("resetAIData()");
    AIData.remain = 100000;
    // remove all invalid number
    // also check that there are 5040 remains 

    /* TODO 03
     *  your code here
     */

    for(int i=0; i<100000; i++) {
        AIData.possible[i]=true;     // add into possible list
        bool hist[10]={0};
        int target = i;   // example: 1134
        for(int j = 0; j < 5; ++j) {
            int digit = target % 10;
            if(hist[digit]) {  // digit is already exist// remove from possible list
                AIData.possible[i] = 0;
                --AIData.remain;
                break;
            }
            hist[digit] = true;
            target /= 10;
         }
         //if (AIData.possible[i]) possibilities.push_back(i);
     }
    
    
    Serial.print("remain: ");
    Serial.print(AIData.remain);
    Serial.print("\n");
    resetAIGuess();
    AIData.count = 0;
}

void AIGameSetup() {
    quitButton = ButtonCoordinate(192,0,128,37);
    reguessButton = ButtonCoordinate(160,37,128,37);
    for (int i = 0; i < 10; ++i) {
        numButtons[i] = ButtonCoordinate((i % 5) * 64, 112 + (i / 5) * 64, 64, 64);
    }
    resetAIData();
    randomSeed(millis());
    gameState = reverseMode;
}

void execAIGame() {
    Serial.print("execAIGame\n");
    while(gameState == reverseMode) {
        AIMove();
        drawAIScreen();
        PlayerMove();
        AIFilter();
    }
}

void setup() {
    pinMode(BL_LED, OUTPUT);
    digitalWrite(BL_LED, HIGH);
    Serial.begin(9600);
    Serial.print("Starting...");
    randomSeed(millis());
 
    if (!ts.begin()) {
        Serial.println("Couldn't start touchscreen controller");
        while (true);
    }

    //Menu Setup
    gameState = menuMode;
    startButton = ButtonCoordinate(112,182,100,40);

    //Initialize Display
    tft.begin();
    tft.setRotation(3);

    drawStartScreen();

}

void loop() {
    if(ts.touched()) {
        TS_Point p0 = ts.getPoint();  //Get touch point
        TS_Point p;
        p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
        p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);

        Serial.print("X = "); Serial.print(p.x);
        Serial.print("\tY = "); Serial.print(p.y);
        Serial.print("\n");

        if(startButton.isPressed(p.x, p.y)) {
            AIGameSetup();
            execAIGame();
        }
    }
    if(gameState == endMode) {
        drawStartScreen();
        gameState = menuMode;
    }
}
