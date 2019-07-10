#include <Adafruit_GFX.h>        // Core graphics library
#include <SPI.h>
#include <Wire.h>            // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <queue>
#include <limits.h>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "util.h"
#include "state.h"
#include "myQueue.h"
#include "buttonCoordinate.h"
#include "myMap.h"

#define TOUCH_CS_PIN D3
#define TOUCH_IRQ_PIN D2 

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 386
#define TS_MINY 178
#define TS_MAXX 3949
#define TS_MAXY 3906

// TODO: Random step move for initial state
#define RANDOM_MOVE 10

XPT2046_Touchscreen ts(TOUCH_CS_PIN,TOUCH_IRQ_PIN);

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

TS_Point p;

extern uint8_t circle[];
extern uint8_t x_bitmap[];

typedef unsigned long long KeyType;
typedef unsigned long long HashType;
typedef unordered_map<KeyType, char> StepMap;
typedef unordered_map<KeyType, char>  HeuMap;
typedef unordered_map<KeyType, char>  FMap;

enum GameState{startMode, playerMode, AStarMode, endMode};
GameState gameState;

ButtonCoordinate playerButton, giveupButton, AStarButton;
ButtonCoordinate buttons[9];

vector<Step> steps;
State initState, currentState;
int countAStar;

void getClickedPlace(int& clickedIndex);
void showAStarResult();
void drawGiveupButton();
void gameSetup();
void resetGame();
void drawStartScreen();
void createStartButton();
void initDisplay();
void drawGameScreen();
void drawGameOverScreen(int moves);
void drawAStarGameOverScreen(int moves);
int A_star_search(State state, int cutoff, vector<Step>& steps);

void setup() {
    pinMode(BL_LED,OUTPUT);
    digitalWrite(BL_LED, HIGH);
    Serial.begin(9600);
    Serial.print("Starting...");
    // randomSeed(millis());
    randomSeed(1);
 
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
    if (istouched || gameState == AStarMode) {
        istouched = false;
        TS_Point p0 = ts.getPoint();  //Get touch point

        p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
        p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);

        Serial.print("X = "); Serial.print(p.x);
        Serial.print("\tY = "); Serial.print(p.y);
        Serial.print("\n");
                
        switch(gameState) {
            case startMode:
                if (playerButton.pressed(p.x, p.y)) {
                    gameState = playerMode;
                    resetGame();
                    printState(currentState);
                    drawGiveupButton();

                } else if (AStarButton.pressed(p.x, p.y)) {
                    gameState = AStarMode;
                    resetGame();
                    printState(currentState);

                    tft.setTextColor(RED);
                    tft.setTextSize(2);
                    tft.setCursor(10, 165);
                    tft.print("Solving...");

                    int cutoff = INT_MAX;
                    countAStar = A_star_search(initState, cutoff, steps);

                    tft.setTextColor(BLUE);
                    tft.setCursor(10, 205);
                    Serial.println("Solved");
                    tft.print("Solved.");
                }
                
                break;

            case playerMode:
                if (giveupButton.pressed(p.x, p.y)) {
                    gameState = endMode;
                    drawGameOverScreen(-1);

                } else {
                    int clickedIndex = -1;
                    getClickedPlace(clickedIndex);
                    delay(200);

                    Step step;
                    if (getLegalStep(currentState, clickedIndex, step)) {
                        currentState = takeStep(currentState, step);
                        printState(currentState);
                    } else {
                        printState(currentState);
                        
                        tft.setTextColor(RED);
                        tft.setTextSize(3);
                        tft.setCursor(10, 150);
                        tft.print("Error");
                    }

                    if (currentState == goalState) {
                        delay(1000);
                        gameState = endMode;
                        drawGameOverScreen(currentState.g);
                    } else {
                        drawGiveupButton();
                    }
                }

                break;

            case AStarMode:
                showAStarResult();

                if (currentState == goalState) {
                    printState(currentState);
                    delay(500);
                    drawAStarGameOverScreen(currentState.g);
                }
                else
                    drawAStarGameOverScreen(INT_MAX);

                gameState = endMode;

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

void getClickedPlace(int& clickedIndex) {

    clickedIndex = -1;
    for (int i = 0; i < 9; ++i) {
        if (buttons[i].pressed(p.x, p.y)) {
            clickedIndex = i;
            tft.fillRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, RED);
            tft.setTextColor(BLUE);
            tft.setTextSize(6);
            printNumber(currentState, i);
            break;
        }
    }
}

void showAStarResult() {
    for (int i = 0; i < steps.size(); ++i) {
        printState(currentState);
        delay(500);

        int p1 = steps[i].p1;
        int p2 = steps[i].p2;
        tft.fillRect(buttons[p1].x, buttons[p1].y, buttons[p1].width, buttons[p1].height, RED);
        tft.fillRect(buttons[p2].x, buttons[p2].y, buttons[p2].width, buttons[p2].height, RED);
        tft.setTextColor(BLUE);
        tft.setTextSize(6);
        printNumber(currentState, p1);
        printNumber(currentState, p2);

        currentState = takeStep(currentState, steps[i]);
    }
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
    playerButton = ButtonCoordinate(30,180,120,40);
    giveupButton = ButtonCoordinate(10,200,80,30);
    AStarButton  = ButtonCoordinate(170,180,120,40);
    for (int i = 0; i < 9; ++i) {
        buttons[i] = ButtonCoordinate(110 + (i % 3) * 70, 30 + (i / 3) * 70, 65, 65);
    }

    for (int i = 1; i <= 8; ++i) {
        goalState.pos[i-1] = i;
    }
    goalState.pos[8] = 0;
    goalState.h = 0;
    goalState.g = 0;
    goalState.f = 0;


}

void resetGame() {
    do {
        initState = randomStepState(RANDOM_MOVE);
        // initState = randomState();
    } while (initState == goalState);
    currentState = initState;

    countAStar = 0;
    steps.clear();
}

void drawStartScreen() {
    tft.fillScreen(BLACK);

    //Draw white frame
    tft.drawRect(0,0,319,240,WHITE);
    
    //Print "8 Puzzle" Text
    tft.setCursor(30,100);
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("8 Puzzle");
    
    //Print "Arduino" Text
    tft.setCursor(80,30);
    tft.setTextColor(GREEN);
    tft.setTextSize(4);
    tft.print("Arduino");

    createStartButton();

}

void createStartButton() {
    //Create Red Button
    //tft.fillRect(30,180,120,40,RED);
    //tft.drawRect(30,180,120,40,WHITE);
    playerButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(36,188);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Player");

    //tft.fillRect(170,180,120,40,RED);
    //tft.drawRect(170,180,120,40,WHITE);
    AStarButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(176,188);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("A star");
}

void initDisplay() {
//  tft.reset();
    tft.begin();
    tft.setRotation(3);
}

void drawGameScreen() {
     tft.fillScreen(BLACK);

     //Draw frame
     tft.drawRect(0,0,319,240,WHITE);

     drawVerticalLine(125);

     drawVerticalLine(195);

     drawHorizontalLine(80);

     drawHorizontalLine(150);
}

void drawGameOverScreen(int moves) {
     
    tft.fillScreen(BLACK);

    //Draw frame
    tft.drawRect(0,0,319,240,WHITE);

    if (moves >= 0) {
        tft.setCursor(100,30);
        tft.setTextColor(RED);
        tft.setTextSize(3);
        tft.print("You Won!");

        tft.setCursor(10,100);
        tft.setTextColor(WHITE);
        tft.setTextSize(3);
        tft.print("You Spent:");
        tft.setCursor(10,130);
        tft.print(" ");
        tft.print(myToChars(moves));
        tft.print(" Moves.");
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

void drawAStarGameOverScreen(int moves) {
     
    tft.fillScreen(BLACK);

    //Draw frame
    tft.drawRect(0,0,319,240,WHITE);

    tft.setCursor(100,30);
    tft.setTextColor(RED);
    tft.setTextSize(3);
    if (moves == INT_MAX)
        tft.print("You Loss");
    else
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
    tft.print(myToChars_six(countAStar));
    
    tft.setCursor(50,220);
    tft.setTextColor(BLUE);
    tft.setTextSize(2);
    tft.print("Click to Continue...");
}


int A_star_search(State state, int cutoff, vector<Step>& steps) {
    /* Some functions that may be helpful
     * getAvailableSteps(state, steps)
     *                      : get all available steps for state and store to steps
     * takeStep(state, step): return the new state as the given state takes the give step
     * myQueue.top()        : return state with minimun cost
     * myQueue.pop()        : pop the state with minimum cost from queue
     * myQueue.push(state)  : push state into queue
     * myQueue.find(state)  : return true if state in queue, else return false
     * myQueue.update(state): update the state in queue
     * myMap.find(state)    : return true if state is in myMap, else return false
     * myMap.get(state)     : return step (value) corresponding to the state (key)
     */
    myQueue<State> frontier;
    myQueue<State> explored;
    myMap prevStep; 
    
    frontier.push(state);

    while (!(frontier.empty())) {
        yield();
        // TODO: Pop state from frontier & update explored and frontier
        /**** Write your code here ****/



        /******************************/

        // Case 1: If current state is goal state, trace back all steps
        if (s == goalState) {
            while (prevStep.find(s)) {
                Step step = prevStep.get(s);
                steps.push_back(step);
                s = takeStep(s, step, true);
            }
            reverse(steps.begin(), steps.end());

            break;
        }

        // If current state not goal state, handle prevStep, frontier and queue
        vector<Step> avSteps;
        getAvailableSteps(s, avSteps);

        for (int i = 0; i < avSteps.size(); ++i) {
            Step step = avSteps[i];
            State successor = takeStep(s, step);
            /* Check if the successor should be added to the frontier or update the frontier
             * If successor already explored (i.e. state in explored set), do not do anything
             */
            if (explored.find(successor)) 
                continue;

            if (frontier.find(successor)) {
                /* TODO
                 * If successor is already in frontier,
                 * (BFS/GS/A*) check if successor's g/h/f is smaller.
                 * If so, remove successor in queue & re add it, update frontier and prevStep
                 * You can directly compare 2 states' cost by using "<" operator
                 * i.e. state1 < state2 returns true if state1's f is smaller than state2's f
                 */
                /**** Write your code here ****/




                /******************************/
            } else {
                /* TODO
                 * Add successor in frontier, update frontier and prevStep
                 */
                /**** Write your code here ****/


                /******************************/
            }
        }
    }

    return explored.size();
}
