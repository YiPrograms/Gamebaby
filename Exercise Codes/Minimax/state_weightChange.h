#include <stdlib.h>
#include <vector>
#include <math.h>
#include <string>
#include <Adafruit_ILI9341.h>

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


extern Adafruit_ILI9341 tft;

using namespace std;

struct State {
    bool pos[8][8]; // false: x (red), true: o (blue)
    bool exist[8][8];
};

bool operator== (const State& s1, const State& s2) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (s1.exist[i][j] != s2.exist[i][j])
                return false;
            if (s1.exist[i][j] && (s1.pos[i][j] != s2.pos[i][j]))
                return false;
        }
    }

    return true;
}

bool operator!= (const State& s1, const State& s2) {
    return !(s1 == s2);
}

String toString(const State& s);
State getState(const String& str);
char* myToChars(int i);
char* myToChars_six(int i);
bool inBoard(int x, int y);
int countResult(const State& s);
int availablePlaces(const State& s, bool (&available)[8][8], bool redTurn);
double heuristic(State& s, bool redTurn);
bool isEnd(const State& s);
void showHost(bool host);
void drawHorizontalLine(int y);
void drawVerticalLine(int x);
void printNumber(const State& s, int i, int j);
void printState(State& s, bool redTurn);
State takeStep(const State& s, int i, int j, bool redTurn);
bool randomMove(const State&s, int& mX, int& mY, int nMoves, bool (&available)[8][8]);

String toString(const State& s) {
    String str = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int c = 0;
            c += (s.pos[i][j] ? 1 : 0);
            c += (s.exist[i][j] ? 2 : 0);
            str[i * 8 + j] = 'A' + c;
        }
    }

    return str;
}
State getState(const String& str) {
    State s;

    int mask2 = 2;
    int mask = 1;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int c = str[i * 8 + j] - 'A';
            s.pos[i][j]   = (c & mask)  > 0 ? true : false;
            s.exist[i][j] = (c & mask2) > 0 ? true : false;
        }
    }

    return s;
}

char* myToChars(int i) {
    char buffer [50];
    sprintf (buffer, "%3i", i);
    return buffer;
}

char* myToChars_six(int i) {
    char buffer [50];
    sprintf (buffer, "%6i", i);
    return buffer;
}

bool inBoard(int x, int y) {
    return (x >= 0 && x < 8 && y >= 0 && y < 8);
}

int countResult(const State& s) {
    int result = 0;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            result += (!s.exist[i][j]) ? 0 : (!s.pos[i][j]) ? 1 : -1;

    return result;
}

int countDisks(const State& s) {
    int result = 0;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            result += (!s.exist[i][j]) ? 0 : 1;
    return result;
}

/// Boss code
int squareWeights(const State& s) {
    int result=0;
    int weights[8][8]={
                          {200, -100, 100,  50,  50, 100, -100,  200,},           
                          {-100, -200, -50, -50, -50, -50, -200, -100},
                          {100,  -50, 100,   0,   0, 100,  -50,  100},
                          {50,  -50,   0,   0,   0,   0,  -50,   50},
                          {50,  -50,   0,   0,   0,   0,  -50,   50},
                          {100,  -50, 100,   0,   0, 100,  -50,  100},
                          {-100, -200, -50, -50, -50, -50, -200, -100},
                          {200, -100, 100, 50, 50, 100, -100, 200}};
    
    if (s.exist[0][0] != 0) {
        weights[0][1]  = 0;
        weights[0][2]  = 0;
        weights[0][3]  = 0;
        weights[1][0]  = 0;
        weights[1][1]  = 0;
        weights[1][2] = 0;
        weights[1][3] = 0;
        weights[2][0] = 0;
        weights[2][1] = 0;
        weights[2][2] = 0;
        weights[3][0] = 0;
        weights[3][1] = 0;
    }

    if (s.exist[0][7] != 0) {
        weights[0][4]  = 0;
        weights[0][5]  = 0;
        weights[0][6]  = 0;
        weights[1][4] = 0;
        weights[1][5] = 0;
        weights[1][6] = 0;
        weights[1][7] = 0;
        weights[2][5] = 0;
        weights[2][6] = 0;
        weights[2][7] = 0;
        weights[3][6] = 0;
        weights[3][7] = 0;
    }

    if (s.exist[7][0] != 0) {
        weights[4][0] = 0;
        weights[4][1] = 0;
        weights[5][0] = 0;
        weights[5][1] = 0;
        weights[5][2] = 0;
        weights[6][0] = 0;
        weights[6][1] = 0;
        weights[6][2] = 0;
        weights[6][3] = 0;
        weights[7][1] = 0;
        weights[7][2] = 0;
        weights[7][3] = 0;
    }

    if (s.exist[7][7] != 0) {
        weights[4][6] = 0;
        weights[4][7] = 0;
        weights[5][5] = 0;
        weights[5][6] = 0;
        weights[5][7] = 0;
        weights[6][4] = 0;
        weights[6][5] = 0;
        weights[6][6] = 0;
        weights[6][7] = 0;
        weights[7][4] = 0;
        weights[7][5] = 0;
        weights[7][6] = 0;
}
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++) {
            result+=(!s.exist[i][j])? 0: (!s.pos[i][j])? 1*weights[i][j]: -1*weights[i][j];
        }
    }
    return result;
}

float boss(const State& s) {
    float result=0;
    float weight[8][8]={{15.0, 1.0, 2.0, 1.5, 1.5, 2.0, 1.0, 15.0},
                        {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                        {2.0, 1.0, 1.2, 1.0, 1.0, 1.2, 1.0, 2.0},
                        {1.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.5},
                        {1.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.5},
                        {2.0, 1.0, 1.2, 1.0, 1.0, 1.2, 1.0, 2.0},
                        {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                        {15.0, 1.0, 2.0, 1.5, 1.5, 2.0, 1.0, 15.0}};
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++) {
            result+=(!s.exist[i][j])? 0: (!s.pos[i][j])? 1*weight[i][j]: -1*weight[i][j];
        }
    }
    return result*(random(1000)+200);
}

int corners(State& s, bool redTurn) {
    int redC = ((s.exist[0][0] && s.pos[0][0])? 1: 0)
              +((s.exist[0][0] && s.pos[0][0])? 1: 0)
              +((s.exist[0][0] && s.pos[0][0])? 1: 0)
              +((s.exist[0][0] && s.pos[0][0])? 1: 0);
    int blueC = ((s.exist[0][0] && !s.pos[0][0])? 1: 0)
              +((s.exist[0][0] && !s.pos[0][0])? 1: 0)
              +((s.exist[0][0] && !s.pos[0][0])? 1: 0)
              +((s.exist[0][0] && !s.pos[0][0])? 1: 0);
    
    if (redTurn) {
        return 100 * (redC - blueC)
            / (redC + blueC + 1);
    }
    else {
        return 100 * (blueC - redC)
            / (redC + blueC + 1);
    }
}

double mobility(int rM, int bM, bool isRed) {
    double m;
    if (!isRed) {
        int tmp=rM;
        rM=bM;
        bM=tmp;
    }
    if(rM > bM)
        m = (100.0 * rM)/(rM + bM);
    else if(rM < bM)
        m = -(100.0 * bM)/(rM + bM);
    else m = 0;
    return m;
}

int availablePlaces(const State& s, bool (&available)[8][8], bool redTurn) {

    bool currentC = redTurn ? false : true;
    bool opponentC = !currentC;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            available[i][j] = false;
            if (s.exist[i][j]) {
                continue;
            }

            // direction (a, b)
            for (int a = -1; a <= 1; ++a) {
                for (int b = -1; b <= 1; ++b) {
                    if (a == 0 && b == 0)
                        continue;

                    bool findOpponent = false;
                    int nStep = 1;
                    while (true) {
                        int x = i + nStep * a, y = j + nStep * b;
                        if (!(inBoard(x, y)))
                            break;

                        if (!s.exist[x][y])
                            break;

                        if (s.pos[x][y] == opponentC)
                            findOpponent = true;

                        if (s.pos[x][y] == currentC) {
                            available[i][j] = findOpponent;
                            break;
                        }

                        ++nStep;
                    }

                    if (available[i][j])
                        break;
                }
                if (available[i][j])
                    break;
            }
        }
    }
    
    int count = 0;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            if (available[i][j])
                ++count;

    return count;

}

double parity(State& s) {
    int squaresRemaining = 64 - countDisks(s);
    if (squaresRemaining % 2 == 0) {
        return -1;
    }
    else {
        return 1;
    }
}

double heuristic(State& s, bool redTurn) {
    // Default heuristic
    bool available[8][8];
    int redMoves = availablePlaces(s, available, true);
    int blueMoves = availablePlaces(s, available, false);

    int diff=abs(countResult(s));
    int cnt=countDisks(s);
    
    if (cnt <= 20) {
        // Opening game
        return 500*mobility(redMoves, blueMoves, redTurn)
            + 10000*squareWeights(s)
            + 1000*corners(s, redTurn)
            + 100*boss(s);
           // + 10000*stability(s, color);
    }
    else if (cnt <= 58) {
        // Midgame
        return 1000*diff
            + 200*mobility(redMoves, blueMoves, redTurn)
            + 30000*squareWeights(s)
            + 1000*parity(s)
            + 10000*corners(s, redTurn)
            + 90*boss(s);
            //+ 10000*stability(board, color);
    }
    else {
        // Endgame
        return 1000*diff
            + 300*parity(s)
            + 500*corners(s, redTurn)
            + 100*boss(s);
            //+ 10000*stability(board, color);
}
    /*
    double h = 0;
    
    
    double h1 = redMoves - blueMoves;
    double h2 = countResult_weighted(s);
    
    // TODO: design your heuristic function
    /**** Write your code here ****
    randomSeed(millis());
    double weight1 = random(10000);
    double weight2 = random(10000)*3;
    h = weight1*h1 + weight2*h2;
    
    /******************************
    
    return h;*/
}

bool isEnd(const State& s) {

    bool available[8][8];
    int redMoves = availablePlaces(s, available, true);
    int blueMoves = availablePlaces(s, available, false);
    return (redMoves == 0 && blueMoves == 0);
}

void showHost(bool host) {

    tft.setCursor(10, 125);
    tft.setTextSize(2);
    if (host) {
        tft.setTextColor(RED);
        tft.print("I'm\n Host");
    } else {
        tft.setTextColor(BLUE);
        tft.print("I'm\n Client");
    }
}

void drawVerticalLine(int x) {
    for(int i=0;i<1;i++) {
        tft.drawLine(x+i,10,x+i,234,WHITE);
    }
}

void drawHorizontalLine(int y) {
    for(int i=0;i<1;i++) {
        tft.drawLine(90,y+i,314,y+i,WHITE);
    }
}

void printNumber(const State& s, int i, int j) {
    if (!s.exist[i][j])
        return;

    int x = 90 + 5 + i * 28;
    int y = 10 + 2 + j * 28;
    tft.setCursor(x, y);
    if (!s.pos[i][j]) {
        tft.setTextColor(RED);
        tft.print("x");
    }
    else {
        tft.setTextColor(BLUE);
        tft.print("o");
    }
    
}

void printState(State& s, bool redTurn) {

    tft.fillScreen(BLACK);
    tft.setTextSize(3);

    for (int i = 0; i < 9; ++i)
        drawVerticalLine(90 + i * 28);

    for (int j = 0; j < 9; ++j)
        drawHorizontalLine(10 + j * 28);

    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            printNumber(s, i, j);

    tft.setTextColor(CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 25);
    tft.print("h:");
    tft.print(myToChars((int)(heuristic(s, redTurn))));

    tft.setCursor(10, 65);
    tft.setTextSize(2);
    if (redTurn) {
        tft.setTextColor(RED);
        tft.print("RED\n Turn");
    } else {
        tft.setTextColor(BLUE);
        tft.print("BLUE\n Turn");
    }
}

State takeStep(const State& s, int i, int j, bool redTurn) {
    State newS = s;

    bool currentC = redTurn ? false : true;
    bool opponentC = !currentC;

    // direction (a, b)
    for (int a = -1; a <= 1; ++a) {
        for (int b = -1; b <= 1; ++b) {
            if (a == 0 && b == 0)
                continue;

            bool check = false;
            bool findOpponent = false;
            int nStep = 1;
            while (true) {
                int x = i + nStep * a, y = j + nStep * b;
                if (!(inBoard(x, y)))
                    break;

                if (!s.exist[x][y])
                    break;

                if (newS.pos[x][y] == opponentC)
                    findOpponent = true;

                if (newS.pos[x][y] == currentC) {
                    check = findOpponent;
                    break;
                }

                ++nStep;
            }

            if (check) {
                for (int k = 1; k < nStep; ++k) {
                    int x = i + k * a, y = j + k * b;
                    newS.pos[x][y] = currentC;
                }
            }
        }
    }
    newS.exist[i][j] = true;
    newS.pos[i][j] = currentC;

    return newS;
}

bool randomMove(const State&s, int& mX, int& mY, int nMoves, bool (&available)[8][8]) {
    
    Serial.println("In randomMove");
    if (nMoves <= 0) {
        Serial.println("It's endState");
        return false;
    }
    size_t k = rand() % nMoves;
    size_t ith = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (available[i][j]) {
                if (ith == k) {
                    mX = i;
                    mY = j;
                    return true;
                }
                ++ith;
            }
        }
    }

    Serial.println("Didn't found kth move");
    return false;
}
