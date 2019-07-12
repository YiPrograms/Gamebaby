#include <Adafruit_GFX.h>        // Core graphics library
#include <SPI.h>
#include <Wire.h>            // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <ESP8266WiFi.h>

#include <queue>
#include <limits.h>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "state.h"
#include "buttonCoordinate.h"

#define TOUCH_CS_PIN D3
#define TOUCH_IRQ_PIN D2 

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 386
#define TS_MINY 178
#define TS_MAXX 3949
#define TS_MAXY 3906

XPT2046_Touchscreen ts(TOUCH_CS_PIN,TOUCH_IRQ_PIN);

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define PAUSE    0

int melody[] = {
  NOTE_DS5, NOTE_E5, NOTE_FS5, 0, NOTE_B5, NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_FS5, NOTE_B5, NOTE_DS6, NOTE_E6, NOTE_DS6, NOTE_AS5, NOTE_B5, 0,
  NOTE_FS5, 0, NOTE_DS5, NOTE_E5, NOTE_FS5, 0, NOTE_B5, NOTE_CS6, NOTE_AS5, NOTE_B5, NOTE_CS6, NOTE_E6, NOTE_DS6, NOTE_E6, NOTE_CS6,
  NOTE_FS4, NOTE_GS4, NOTE_D4, NOTE_DS4, NOTE_FS2, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_CS4,
  NOTE_D4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_CS4, NOTE_B3,
  NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_D4, NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4,
  NOTE_D4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_B3, NOTE_CS4,
  NOTE_FS4, NOTE_GS4, NOTE_D4, NOTE_DS4, NOTE_FS2, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_CS4,
  NOTE_D4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_CS4, NOTE_B3,
  NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_D4, NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4,
  NOTE_D4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_B3, NOTE_B3,
  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4,
  NOTE_B3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_E4, NOTE_DS4, NOTE_CS4, NOTE_B3, NOTE_E3, NOTE_DS3, NOTE_E3, NOTE_FS3,
  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_FS3,
  NOTE_B3, NOTE_B3, NOTE_AS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4, NOTE_B3, NOTE_AS3,
  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4,
  NOTE_B3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_E4, NOTE_DS4, NOTE_CS4, NOTE_B3, NOTE_E3, NOTE_DS3, NOTE_E3, NOTE_FS3,
  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_FS3,
  NOTE_B3, NOTE_B3, NOTE_AS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4, NOTE_B3, NOTE_CS4,
  NOTE_FS4, NOTE_GS4, NOTE_D4, NOTE_DS4, NOTE_FS2, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_CS4,
  NOTE_D4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_CS4, NOTE_B3,
  NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_D4, NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4,
  NOTE_D4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_B3, NOTE_CS4,
  NOTE_FS4, NOTE_GS4, NOTE_D4, NOTE_DS4, NOTE_FS2, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_CS4,
  NOTE_D4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_CS4, NOTE_B3,
  NOTE_DS4, NOTE_FS4, NOTE_GS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_D4, NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4,
  NOTE_D4, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_FS4, NOTE_CS4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_B3, NOTE_B3,
  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4,
  NOTE_B3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_E4, NOTE_DS4, NOTE_CS4, NOTE_B3, NOTE_E3, NOTE_DS3, NOTE_E3, NOTE_FS3,
  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_FS3,
  NOTE_B3, NOTE_B3, NOTE_AS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4, NOTE_B3, NOTE_AS3,
  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4,
  NOTE_B3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_E4, NOTE_DS4, NOTE_CS4, NOTE_B3, NOTE_E3, NOTE_DS3, NOTE_E3, NOTE_FS3,
  NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_B3, NOTE_CS4, NOTE_DS4, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_FS3,
  NOTE_B3, NOTE_B3, NOTE_AS3, NOTE_B3, NOTE_FS3, NOTE_GS3, NOTE_B3, NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_FS4, NOTE_B3, NOTE_CS4,
};

// note durations: 4 = quarter note, 8 = eighth note, etc
int noteDurations[] = {
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,8,16,16,16,16,16,16,16,16,
  8,8,16,16,16,16,16,16,8,8,8,
  8,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
  8,16,16,16,16,16,16,16,16,8,8,8,
  8,8,16,16,16,16,16,16,8,8,8,
  8,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
  8,16,16,16,16,16,16,16,16,8,8,8,
  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
  8,16,16,16,16,16,16,16,16,16,16,8,8,
  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
  8,16,16,16,16,16,16,16,16,16,16,8,8,
  8,8,16,16,16,16,16,16,8,8,8,
  8,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
  8,16,16,16,16,16,16,16,16,8,8,8,
  8,8,16,16,16,16,16,16,8,8,8,
  8,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
  8,16,16,16,16,16,16,16,16,8,8,8,
  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
  8,16,16,16,16,16,16,16,16,16,16,8,8,
  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
  8,8,16,16,16,16,16,16,16,16,16,16,16,16,
  8,16,16,8,16,16,16,16,16,16,16,16,16,16,
  8,16,16,16,16,16,16,16,16,16,16,8,8,
};

// The display also uses hardware SPI, plus #9 & #10
static uint8_t SD3 = 10;
#define TFT_CS SD3
#define TFT_DC D4
#define BL_LED D8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define HOSTNAME "ESP-"

TS_Point p;

WiFiServer server(80);
WiFiClient client;
const char* defaultIP = "192.168.4.1";

const char* espSSID = "ESP-5A12F8";
const char* espPASS = "NTUEE_AI";

const String startPattern = "Start:";

extern uint8_t circle[];
extern uint8_t x_bitmap[];

typedef unordered_map<unsigned long long, int>  HeuMap;

enum GameState{startMode, playerMode, PvEMode, MinimaxMode, HostWaitMode, HostMode, ClientConnectMode, ClientMode, endMode};
GameState gameState;

ButtonCoordinate playerButton, PvEButton, MinimaxButton, HostButton, ClientButton, giveupButton, resetButton;
ButtonCoordinate buttons[8][8], ssidButtons[10];
String ssids[10];

State initState, currentState;
int countMinimax;
bool availables[8][8];
bool redTurn;
int moveX, moveY;
const int maxDepth = 5;

// Time: milli second as unit
unsigned long deadline;
const unsigned long maxPeriod = 2000; // 2 seconds at most
const unsigned long bufferTime = 50; // 0.1 second as buffer

void getClickedPlace(int& clickedI, int& clickedJ);
void applyMove(int clickedI, int clickedJ);
void envMove();
bool isEndState();
String readSignal();
bool updateState(String signal);
String createSignal();
void hostSetup();
void clientSetup();
void drawResetButton();
void drawGiveupButton();
void gameSetup();
void resetGame();
void drawStartScreen();
void createStartButton();
void initDisplay();
void drawGameOverScreen(int result);
void drawMinimaxGameOverScreen(int moves);
bool reachDeadline(bool withBuffer = true);
double minimax(const State& s, int& mX, int& mY, bool redTurn, int depth, int alpha = INT_MIN, int beta = INT_MAX);

int thisNote;

void setup() {
    thisNote=0;
    pinMode(BL_LED,OUTPUT);
    digitalWrite(BL_LED, HIGH);
    Serial.begin(9600);
    Serial.print("Starting...");
    randomSeed(millis());
 
    if (!ts.begin()) {
        Serial.println("Couldn't start touchscreen controller");
        while (1);
    }

    gameSetup();
    initDisplay();
    drawStartScreen();
}



void loop() {
    boolean istouched = ts.touched();
    if ((thisNote < sizeof(melody) / sizeof(melody[0])) && !istouched){
        int noteDuration = 1000/noteDurations[thisNote];
        tone(D1, melody[thisNote]*2,noteDuration);

        int pauseBetweenNotes = noteDuration * 1.2;
        delay(pauseBetweenNotes);
    
        noTone(D1);
        thisNote++;
        if (!(thisNote < sizeof(melody) / sizeof(melody[0]))) thisNote=0;
    }
    
    if (istouched || (gameState == PvEMode && !redTurn) ||
            gameState == MinimaxMode ||
            gameState == HostWaitMode ||
            gameState == HostMode ||
            gameState == ClientMode) {

        TS_Point p0 = ts.getPoint();  //Get touch point

        p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
        p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);

        if (istouched) {
            Serial.print("X = "); Serial.print(p.x);
            Serial.print("\tY = "); Serial.print(p.y);
            Serial.print("\n");
        }
        int clickedI = -1, clickedJ = -1;
                
        switch(gameState) {
            case startMode:
                if (playerButton.pressed(p.x, p.y)) {
                    resetGame();
                    printState(currentState, redTurn);
                    gameState = playerMode;
                    drawGiveupButton();
                } else if (PvEButton.pressed(p.x, p.y)) {
                    resetGame();
                    printState(currentState, redTurn);
                    gameState = PvEMode;
                    drawGiveupButton();
                } else if (MinimaxButton.pressed(p.x, p.y)) {
                    resetGame();
                    printState(currentState, redTurn);
                    gameState = MinimaxMode;
                } else if (HostButton.pressed(p.x, p.y)) {
                    hostSetup();
                    gameState = HostWaitMode;
                } else if (ClientButton.pressed(p.x, p.y)) {
                    clientSetup();
                    drawResetButton();
                    gameState = ClientConnectMode;
                }

                break;

            case playerMode:
                if (giveupButton.pressed(p.x, p.y)) {
                    gameState = endMode;
                    drawGameOverScreen(INT_MIN);

                    return;
                }
                getClickedPlace(clickedI, clickedJ);
                delay(200);
                applyMove(clickedI, clickedJ);
                if (isEndState()) {
                    delay(1000);
                    gameState = endMode;
                    drawGameOverScreen(countResult(currentState));
                } else {
                    drawGiveupButton();
                }


                break;

            case PvEMode:
                if (!redTurn) {
                    envMove();
                    if (isEndState()) {
                        delay(1000);
                        gameState = endMode;
                        drawGameOverScreen(countResult(currentState));
                    } else {
                        drawGiveupButton();
                    }
                    return;
                }
                if (giveupButton.pressed(p.x, p.y)) {
                    gameState = endMode;
                    drawGameOverScreen(INT_MIN);

                    return;
                }
                getClickedPlace(clickedI, clickedJ);
                delay(200);
                applyMove(clickedI, clickedJ);
                if (isEndState()) {
                    delay(1000);
                    gameState = endMode;
                    drawGameOverScreen(countResult(currentState));
                } else {
                    drawGiveupButton();
                }

                break;

            case MinimaxMode:
                envMove();
                if (isEndState()) {
                    delay(1000);
                    gameState = endMode;
                    drawGameOverScreen(countResult(currentState));
                }

                break;

            case HostWaitMode:
                if (istouched && resetButton.pressed(p.x, p.y)) {
                    tft.print ("reset pressed");
                    WiFi.mode(WIFI_OFF);
                    gameState = startMode;
                    drawStartScreen();
                    return;
                }

                client = server.available();
                if (client && client.connected()) {
                    tft.print ("client connected");
                    resetGame();
                    printState(currentState, redTurn);
                    gameState = HostMode;
                }

                break;

            case ClientConnectMode:
                if (istouched && resetButton.pressed(p.x, p.y)) {
                    gameState = startMode;
                    drawStartScreen();
                    return;
                }

                clickedI = -1;
                for (int i = 0; i < 10; ++i) {
                    if (ssidButtons[i].pressed(p.x, p.y)) {
                        clickedI = i;
                        break;
                    }
                }
                if (clickedI >= 0 && ssids[clickedI].substring(0, 3) == "ESP") {
                    do {
                        Serial.print ("Connecting to ");
                        Serial.println (ssids[clickedI].c_str());
                        WiFi.begin(ssids[clickedI].c_str(), espPASS);
                        delay(5000);
                    } while (WiFi.status() != WL_CONNECTED);
                    
                    tft.setTextColor(RED);
                    if (client.connect(defaultIP, 80)) {
                        tft.print ("Connected to server ");
                        resetGame();
                        printState(currentState, redTurn);
                        gameState = ClientMode;
                    } else {
                        tft.print("Fail to Connect");
                    }
                }

                break;
            case HostMode:
            case ClientMode:
                if (istouched && resetButton.pressed(p.x, p.y)) {
                    WiFi.mode(WIFI_OFF);
                    gameState = startMode;
                    drawStartScreen();
                    return;
                }
                if (!(client && client.connected())) {
                    Serial.println("The client disconnected");
                    tft.println("The client disconnected");
                    delay(3000);
                    gameState = endMode;
                    drawGameOverScreen(countResult(currentState));
                    return;
                }
                if (redTurn == (gameState == HostMode)) {
                    envMove();
                    showHost(gameState == HostMode);
    
                    String signal = createSignal();
                    client.println(startPattern + signal);
    
                    if (isEndState()) {
                        delay(1000);
                        gameState = endMode;
                        drawGameOverScreen(countResult(currentState));
                    }
                } else {
                    String signal = "";
                    while (client.connected() && (signal = readSignal()) == "") {
                        delay(200);
                    }
                    if (!updateState(signal)) {
                        tft.println ("Something's wrong, aborting...");
                        if (client.connected())
                            client.println(startPattern); // Error message
                        delay(5000);
                        WiFi.mode(WIFI_OFF);
                        gameState = endMode;
                        drawGameOverScreen(countResult(currentState));
                    } else {
                        printState(currentState, redTurn);
                        showHost(gameState == HostMode);
    
                        if (isEndState()) {
                            delay(1000);
                            gameState = endMode;
                            drawGameOverScreen(countResult(currentState));
                        }
                    }
                }

                break;

            case endMode:
                gameState = startMode;
                drawStartScreen();

                break;

            default:
                Serial.println("Unknown GameState:");
                Serial.println(gameState);

                break;
        }

        delay(10);  
    }
}

bool isEndState() {
    return availablePlaces(currentState, availables, redTurn) == 0 && availablePlaces(currentState, availables, !redTurn) == 0;
}

void applyMove(int clickedI, int clickedJ) {

    availablePlaces(currentState, availables, redTurn);
    if (!(clickedI == -1 || clickedJ == -1) && availables[clickedI][clickedJ]) {
        currentState = takeStep(currentState, clickedI, clickedJ, redTurn);
        redTurn = !redTurn;

        if (availablePlaces(currentState, availables, redTurn) == 0) {
            redTurn = !redTurn;
        }
        printState(currentState, redTurn);
    } else {
        printState(currentState, redTurn);
        tft.setTextColor(RED);
        tft.setTextSize(2);
        tft.setCursor(10, 150);
        tft.print("Error");
    }
}

void envMove() {

    int nMoves = availablePlaces(currentState, availables, redTurn);
    moveX = moveY = -1;
    deadline = millis() + maxPeriod;
    double returnValue = minimax(currentState, moveX, moveY, redTurn, maxDepth);
    if ((moveX != -1 && reachDeadline(false) == false) || randomMove(currentState, moveX, moveY, nMoves, availables)) {
        Serial.print ("moveX: ");
        Serial.print (moveX);
        Serial.print (", moveY: ");
        Serial.print (moveY);
        Serial.print (", return value: ");
        Serial.println (returnValue);
        currentState = takeStep(currentState, moveX, moveY, redTurn);
        redTurn = !redTurn;
    } else {
        Serial.println("Something went wrong QQ");
    }

    if (availablePlaces(currentState, availables, redTurn) == 0) {
        redTurn = !redTurn;
    }
    printState(currentState, redTurn);
}

void getClickedPlace(int& clickedI, int& clickedJ) {

    clickedI = -1, clickedJ = -1;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (buttons[i][j].pressed(p.x, p.y)) {
                clickedI = i;
                clickedJ = j;
                tft.fillRect(buttons[i][j].x, buttons[i][j].y, buttons[i][j].width, buttons[i][j].height, GREEN);
                tft.setTextSize(3);
                printNumber(currentState, i, j);
                break;
            }
        }
    }
}

String readSignal() {
    char c;
    String data = "";
    while (client.connected()) {
        if ((c = (char)client.read()) != -1) {
            data += c;
        } else {
            delay(100);
        }
        if (c == '\n') {
            break;
        }
        delay(10);
    }
    int index = data.indexOf(startPattern);
    if (index == -1)
        return "";

    return data.substring(index + startPattern.length());
}

/*
String sendSignal(WiFiClient& client, String signal) {
    client.println(startPattern + signal);
}
*/

// false: something's wrong
bool updateState(String signal) {
    bool rTurn;
    int index, mX, mY;
    State newS;

    index = signal.indexOf("RT:");
    if (index == -1)
        return false;
    rTurn = signal[index+3] == 'T';
    index = signal.indexOf("MV:");
    if (index == -1)
        return false;
    mX = (int)(signal[index+3] - '0');
    mY = (int)(signal[index+4] - '0');
    index = signal.indexOf("ST:");
    if (index == -1)
        return false;
    newS = getState(signal.substring(index+3, index+3+64));
    currentState = takeStep(currentState, mX, mY, redTurn);
    if (currentState != newS) {
        Serial.println ("state not match: ");
        Serial.println (signal.substring(index, index+64));
        tft.println ("state not match!");
        delay(2000);
        printState(newS, redTurn);
        return false;
    }

    redTurn = !redTurn;

    if (availablePlaces(currentState, availables, redTurn) == 0) {
        redTurn = !redTurn;
    }

    if (redTurn != rTurn) {
        Serial.println ("redTurn not match!");
        tft.println ("redTurn not match!");
        return false;
    }

    return true;
}

String createSignal() {
    String signal = "";
    signal += "RT:";
    if (redTurn)
        signal += "T";
    else
        signal += "F";
    signal += ",MV:";
    signal = String(signal + (char)('0' + moveX));
    signal = String(signal + (char)('0' + moveY));
    signal += ",ST:";

    String stateStr = toString(currentState);
    signal += stateStr;

    return signal;
}

void hostSetup() {
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.setTextSize(3);
    tft.setCursor(10, 10);
    tft.print("Waiting for \nconnection...");

    WiFi.mode(WIFI_AP);
    String hostname(HOSTNAME);
    String id = String(ESP.getChipId(), HEX);
    id.toUpperCase();
    hostname += id;
    // local ip: 192.168.4.1
    // IPAddress local_IP(192,168,4,22);
    // IPAddress gateway(192,168,4,9);
    // IPAddress subnet(255,255,255,0);

    bool result = WiFi.softAP(hostname.c_str(), espPASS);
    if(result == true) {
        Serial.println("Ready");
        tft.setCursor(10, 80);
        tft.print("My SSID:");
        tft.setCursor(30, 110);
        tft.print(hostname);
        tft.setCursor(10, 140);
        tft.print("My softAPIP:");
        tft.setCursor(30, 170);
        tft.print(WiFi.softAPIP());
        server.begin();
        drawResetButton();
    }
    else {
        Serial.println("Failed!");
        tft.setCursor(10, 50);
        tft.print("Fail to establish AP");
        delay(2000);

        WiFi.softAPdisconnect(true);
        gameState = startMode;
        drawStartScreen();
    }

}

void clientSetup() {
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.setCursor(10, 10);
    tft.print("Choose a network");

    WiFi.mode(WIFI_STA);

    int n = WiFi.scanNetworks();

    for (int i = 0; i < 10; ++i)
        ssidButtons[i].fillAndDraw(tft, BLACK, WHITE);

    tft.setTextSize(2);
    for (int i = 0; i < min(10, n); ++i) {
        ssids[i] = WiFi.SSID(i);
        tft.setCursor(ssidButtons[i].x + 5, ssidButtons[i].y + 5);
        tft.print(ssids[i].substring(0, 10));
    }
}

void drawResetButton() {
    resetButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(resetButton.x + 2, resetButton.y + 2);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("reset");
}

void drawGiveupButton() {
    giveupButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(giveupButton.x + 5, giveupButton.y + 5);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Giveup");
}

void gameSetup() {
    gameState = startMode;
    playerButton = ButtonCoordinate(10,180,90,40);
    PvEButton = ButtonCoordinate(110,180,90,40);
    MinimaxButton  = ButtonCoordinate(210,180,90,40);

    HostButton  = ButtonCoordinate(30,130,120,40);
    ClientButton  = ButtonCoordinate(170,130,120,40);

    resetButton = ButtonCoordinate(250, 50, 70, 25);
    giveupButton = ButtonCoordinate(5,200,80,30);
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            buttons[i][j] = ButtonCoordinate(90 + 1 + i * 28, 10 + 1 + j * 28, 27, 27);

    for (int i = 0; i < 10; ++i) {
        ssidButtons[i] = ButtonCoordinate(10 + (i/5) * 160, 75 + (i % 5) * 33, 140, 30);
    }

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            initState.exist[i][j] = false;
            initState.pos[i][j] = false;
        }
    }
    initState.exist[3][3] = true;
    initState.exist[4][4] = true;
    initState.exist[3][4] = true;
    initState.exist[4][3] = true;

    initState.pos[3][3] = true;
    initState.pos[4][4] = true;
    initState.pos[3][4] = false;
    initState.pos[4][3] = false;
    
    /*
    for (int i = 1; i < 8; ++i) {
        for (int j = 1; j < 8; ++j) {
            initState.pos[i][j] = (i+j) % 2 == 1 ? true : false;
            initState.exist[i][j] = true;
        }
    }
    */
}

void resetGame() {
    currentState = initState;

    countMinimax = 0;
    redTurn = true;
    availablePlaces(currentState, availables, redTurn);
}

void drawStartScreen() {
    tft.fillScreen(BLACK);

    //Draw white frame
    tft.drawRect(0,0,319,240,WHITE);
    
    //Print "8 Puzzle" Text
    tft.setCursor(30,80);
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Reversi");
    
    //Print "Arduino" Text
    tft.setCursor(80,30);
    tft.setTextColor(GREEN);
    tft.setTextSize(4);
    tft.print("Arduino");

    createStartButton();

}

void createStartButton() {
    //Create Red Button
    playerButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(playerButton.x + 10, playerButton.y + 12);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Player");

    PvEButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(PvEButton.x + 6, PvEButton.y + 10);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("P vs E");

    MinimaxButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(MinimaxButton.x + 6, MinimaxButton.y + 10);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Minimax");

    HostButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(HostButton.x + 26, HostButton.y + 10);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Host");

    ClientButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(ClientButton.x + 6, ClientButton.y + 10);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Client");
}

void initDisplay() {
//  tft.reset();
    tft.begin();
    tft.setRotation(3);
}

void drawGameOverScreen(int result) {
     
    tft.fillScreen(BLACK);

    //Draw frame
    tft.drawRect(0,0,319,240,WHITE);

    if (result != INT_MIN) {
        tft.setTextSize(3);
        if (result > 0) {
            tft.setCursor(100,30);
            tft.setTextColor(RED);
            tft.print("Red Win!");
        } else if (result == 0) {
            tft.setCursor(105,30);
            tft.setTextColor(MAGENTA);
            tft.print("Tie Up!");
        } else {
            tft.setCursor(95,30);
            tft.setTextColor(BLUE);
            tft.print("Blue Win!");
        }

        tft.setCursor(10,100);
        tft.setTextColor(WHITE);
        tft.setTextSize(3);
        tft.print("Result: ");
        tft.print(myToChars(result));
    } else {
        tft.setCursor(40,100);
        tft.setTextColor(RED);
        tft.setTextSize(3);
        tft.print("You Giveup QQ");

    }
    
    tft.setCursor(50,220);
    tft.setTextColor(BLUE);
    tft.setTextSize(2);
    tft.print("Click to Continue...");
}

void drawMinimaxGameOverScreen(int moves) {
     
    tft.fillScreen(BLACK);

    //Draw frame
    tft.drawRect(0,0,319,240,WHITE);

    tft.setCursor(100,30);
    tft.setTextColor(RED);
    tft.setTextSize(3);
    tft.print("You Won!");

    tft.setCursor(10,100);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Move :   ");
    tft.print(myToChars(moves));

    tft.setCursor(10,140);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Model:");
    tft.print(myToChars_six(countMinimax));
    
    tft.setCursor(50,220);
    tft.setTextColor(BLUE);
    tft.setTextSize(2);
    tft.print("Click to Continue...");
}

bool reachDeadline(bool withBuffer) {
    if ((gameState == HostMode || gameState == ClientMode)) {
        if (withBuffer) {
            if (millis() >= deadline - bufferTime)
                return true;
        } else {
            if (millis() >= deadline)
                return true;
        }
    } 
 
    return false;
}

// return best heuristic value with a limited trace depth
double minimax(const State& s, int& mX, int& mY, bool redTurn, int depth, int alpha, int beta) {

    mX = -1;
    mY = -1;

    double bestHeu = (redTurn ? -10000000 : 10000000);

    if (reachDeadline()) {
        return bestHeu;
    }

    yield();

    // Generate next available places
    bool newAvailables[8][8];
    availablePlaces(s, newAvailables, redTurn);
    bool finish = false;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (!newAvailables[i][j])
                continue;

            if (reachDeadline()) {
                finish = true;
                break;
            }

            double h;
            State newS = takeStep(s, i, j, redTurn);
            if (depth <= 1 || isEnd(newS)) {
                h = heuristic(newS, redTurn);
            } else {
                int tempMX = -1, tempMY = -1;
                h = minimax(newS, tempMX, tempMY, !redTurn, depth - 1, alpha, beta);
            }

            if (mX == -1 || (redTurn && h > bestHeu) || (!redTurn && h < bestHeu)) {
                mX = i;
                mY = j;
                bestHeu = h;
                if (redTurn && bestHeu > alpha)
                    alpha = bestHeu;
                else if (!redTurn && bestHeu < beta)
                    beta = bestHeu;


                if (alpha >= beta)
                    finish = true;
            }
            if (finish)
                break;
        }
        if (finish)
            break;
    }
    if (mX == -1 && reachDeadline() == false) {
        int tempMX = -1, tempMY = -1;
        bestHeu = minimax(s, tempMX, tempMY, !redTurn, depth);
    }

    return bestHeu;
}
