#include <Adafruit_GFX.h>  // Core graphics library
#include <SPI.h>
#include <Wire.h>  // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <string.h>
#include "buttonCoordinate.h"
#include <vector>


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
 * ILI9341_WHITE   0xFFFFSerial.print("Back");
 */

ButtonCoordinate startButton, giveupButton, testingButton, reguessButton, backButton, backdoorButton;
ButtonCoordinate numButtons[10];

enum GameState{menuMode, gameMode, testingMode, endMode};
GameState gameState;

bool backdoor=false;

struct GameData {
    char R[5];       // the four correct numbers
    char guess[5];   // the four numbers that player provides
    bool histR[10];  // record the four correct numbers by bits
    bool histG[10];  // record the four numbers that player provides by bits
    int A;           // correct numbers at correct position
    int B;           // correct numbers at wrong position
    int count;       // how many times player had guessed
} gameData;

void drawStartScreen() {
    //Clean the screen
    tft.fillScreen(ILI9341_BLACK);

    //Draw white frame
    tft.drawRect(0, 0, 319, 240, ILI9341_WHITE);
    
    //Show buttons
    tft.setCursor(18,100);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(4);
    tft.print("Bulls & Cows");
    
    tft.setCursor(80, 30);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(4);
    tft.print("Arduino");

    startButton.fillAndDraw(tft, ILI9341_RED, ILI9341_WHITE);
    tft.setCursor(36, 188);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.print("Start");
    
    testingButton.fillAndDraw(tft, ILI9341_RED, ILI9341_WHITE);
    tft.setCursor(156, 188);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.print("Testing");
}

void drawGameScreen() {
    tft.fillScreen(ILI9341_BLACK);
    tft.drawRect(0, 0, 319, 240, ILI9341_WHITE);
    for(int i = 0; i < 4; ++i) {
        drawVerticalLine(64 * (i+1) - 2);
    }
    for(int i = 0; i < 4; ++i) {
        drawHorizontalLine(112 + 64 * i - 2);
    }
    for(int i = 0; i < 10; ++i) {
        if(!gameData.histG[i]) printNumber(i);
    }
    tft.setCursor(192, 0 + 8);
    tft.print("Give Up");
    tft.drawRect(185,0,135,35,ILI9341_WHITE);
    tft.setCursor(240, 35 + 8);
    tft.print("<-");
    tft.drawRect(185,35,135,35,ILI9341_WHITE);
    // draw guess
    tft.setCursor(8, 8);
    if (backdoor) tft.print(gameData.R);
    else tft.print(gameData.guess);
    // draw result
    if(gameData.A == 4) {
        tft.setCursor(8, 74 + 8);
        tft.print("Bingo!");
    }
    else {
        tft.setCursor(8, 37 + 8);
        tft.print("A: ");
        if(gameData.A >= 0) tft.print(gameData.A);
        tft.setCursor(8, 74 + 8);
        tft.print("B: ");
        if(gameData.B >= 0) tft.print(gameData.B);
    }
    tft.setCursor(128 + 8, 74 + 8);
    tft.print("Count: ");
    tft.print(gameData.count);
}

void drawInputScreen() {
    tft.fillScreen(ILI9341_BLACK);
    tft.drawRect(0,0,319,240,ILI9341_WHITE);
    for(int i = 0; i < 4; ++i) {
        drawVerticalLine(64 * (i+1) - 2);
    }
    for(int i = 0; i < 4; ++i) {
        drawHorizontalLine(112 + 64 * i - 2);
    }
    for(int i = 0; i < 10; ++i) {
        if(!gameData.histR[i]) printNumber(i);
    }
    tft.setCursor(8, 0 + 8);
    tft.print("Please input");
    tft.setCursor(8, 37 + 8);
    tft.print("correct answer:");
    // draw guess
    tft.setCursor(8, 74 + 8);
    tft.print(gameData.R);
}

void drawVerticalLine(int x) {
    for(int i=0;i<5;i++) {
        tft.drawLine(x+i, 112, x+i, 240, ILI9341_WHITE);
    }
}

void drawHorizontalLine(int y) {
    for(int i=0;i<5;i++) {
        tft.drawLine(0, y+i, 320, y+i, ILI9341_WHITE);
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

void generate() {
    if(gameState == testingMode) {
        drawInputScreen();
        while(ts.touched()) delay(50);
        delay(50);
        int count = 0;
        while(count < 4) {
            if(ts.touched()) {
                if(count == 0) resetGuess();
                TS_Point p0 = ts.getPoint();  //Get touch point
                TS_Point p;
                p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
                p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);
                Serial.print("X = "); Serial.print(p.x);
                Serial.print("\tY = "); Serial.print(p.y);
                Serial.print("\n");
                for(int i = 0; i < 10; ++i) {
                    if(numButtons[i].isPressed(p.x, p.y)) {
                        if(!gameData.histR[i]) {
                            gameData.R[count] = '0' + i;
                            gameData.histR[i] = 1;
                            ++count;
                            drawInputScreen();
                        }
                        Serial.print("\tanswer = ");
                        Serial.print(gameData.guess);
                        Serial.print("\n");
                    }
                }
            }
            delay(50); // prevent wdt reset and continuous input
        }
    }
    else {
        /* TODO 01
         *  your code here
         */
        int a[10];
        for (int i=0; i<10; i++) a[i]=i+1;
        for (int i=0; i<10; i++) {
            int rnd=random(10);
            int tmp=a[i];
            a[i]=a[rnd];
            a[rnd]=tmp;
        }
        for (int i=0; i<4; i++) gameData.R[i]=a[i]+'0';
        Serial.print("generate R: ");
        Serial.print(gameData.R);
        Serial.print("\n");
    }
}

void playerMove() {
    int count = 0;
    while(count < 4) {
        if(ts.touched()) {
            if(count == 0) resetGuess();
            TS_Point p0 = ts.getPoint();  //Get touch point
            TS_Point p;
            p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
            p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);
            Serial.print("X = "); Serial.print(p.x);
            Serial.print("\tY = "); Serial.print(p.y);
            Serial.print("\n");
            if(giveupButton.isPressed(p.x, p.y)) {
                gameState = endMode;
                break;
            }
            if (backButton.isPressed(p.x, p.y)) {
                if (count!=0) {
                    int i=gameData.guess[count-1]-'0';
                    gameData.guess[--count]='x';
                    gameData.histG[i]=false;
                    drawGameScreen();
                }
            }
            if (backdoorButton.isPressed(p.x, p.y)) {
                if (!backdoor) {
                    backdoor=true;
                    drawGameScreen();
                } else {
                    backdoor=false;
                    drawGameScreen();
                }
            }
            for(int i = 0; i < 10; ++i) {
                if(numButtons[i].isPressed(p.x, p.y)) {
                    if(!gameData.histG[i]) {
                        /* TODO 02
                         *  your code here
                         */
                         gameData.guess[count]=i+'0';
                         gameData.histG[i]=true;
                         count++;
                         drawGameScreen();
                    }
                    Serial.print("count = ");
                    Serial.print(count);
                    Serial.print("\tguess = ");
                    Serial.print(gameData.guess);
                    Serial.print("\n");
                }
            }
        }
        delay(50); // prevent wdt reset and continuous input
    }
    ++gameData.count;
}

void checkGuess() {
    Serial.print("checkGuess\n");

    /* TODO 03
     *  your code here
     */
    gameData.A=0;
    gameData.B=0;
    for(int i=0; i<4; i++) {
        if(gameData.R[i]==gameData.guess[i]) gameData.A++;
    }
    for(int i=0; i<10; i++) {
        if(gameData.histR[i]&&gameData.histG[i]) gameData.B++;
    }
    gameData.B-=gameData.A;

    if(gameData.A == 4) {
        gameState = endMode;
        drawGameScreen();
        delay(2000);
    }
    for(int i = 0; i < 10; ++i) {
        gameData.histG[i] = 0;
    }
}

void resetGuess() {
    strcpy(gameData.guess, "xxxx");
    for(int i = 0; i < 10; ++i) {
        gameData.histG[i] = 0;
    }
    gameData.A = -1;
    gameData.B = -1;
}

void resetGameData() {
    resetGuess();
    strcpy(gameData.R, "    ");
    for(int i = 0; i < 10; ++i) {
        gameData.histR[i] = 0;
    }
    gameData.count = 0;
}

void gameSetup() {
    backdoor=false;
    Serial.print("gameSetup\n");
    giveupButton = ButtonCoordinate(192,0,128,32);
    backButton = ButtonCoordinate(200,32,128,32);
    backdoorButton = ButtonCoordinate(8, 8, 100,32);
    for (int i = 0; i < 10; ++i) {
        numButtons[i] = ButtonCoordinate((i % 5) * 64, 112 + (i / 5) * 64, 64, 64);
    }
    resetGameData();
    generate();
    gameState = gameMode;
    Serial.print("finish gameSetup\n");
}

void execGame() {
    Serial.print("execGame\n");
    while(gameState == gameMode) {
        drawGameScreen();
        playerMove();
        checkGuess();
    }
}

void setup() {
    pinMode(BL_LED, OUTPUT);
    digitalWrite(BL_LED, HIGH);
    Serial.begin(115200);
    Serial.print("Starting...");
    randomSeed(millis());
 
    if (!ts.begin()) {
        Serial.println("Couldn't start touchscreen controller");
        while (true);
    }

    //Menu Setup
    gameState = menuMode;
    startButton = ButtonCoordinate(30,180,100,40);
    testingButton  = ButtonCoordinate(150,180,140,40);

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
            gameSetup();
            execGame();
        }
        else if(testingButton.isPressed(p.x, p.y)) {
            gameState = testingMode;
            gameSetup();
            execGame();
        }
    }
    if(gameState == endMode) {
        drawStartScreen();
        gameState = menuMode;
    }
}
