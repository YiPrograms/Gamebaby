#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <Wire.h>  
#include <vector>
using std::vector;

#define TOUCH_CS_PIN D3
#define TOUCH_IRQ_PIN D2

#define TS_MINX 330
#define TS_MINY 213
#define TS_MAXX 3963
#define TS_MAXY 3890

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
XPT2046_Touchscreen ts(TOUCH_CS_PIN,TOUCH_IRQ_PIN);

// The display also uses hardware SPI(D5,D6,D7), SD3, D4, D8
static uint8_t SD3 = 10;
#define TFT_CS SD3
#define TFT_DC D4
#define BL_LED D8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

double answer = 0;
char lastchar = ' ';
String key1;
String key2;
double key1i = 0;
double key2i = 0;
bool key1n=false;
bool key2n=false;
bool key1d=false;
bool key2d=false;
char operation = ' ';
/*long Time = 0;
long millicount = 0;
int interval = 1000;
int screenTimout = 15;*/
bool beentouched = false;
bool toLong = false;
bool equal = false;

#define row1x 0
#define boxsize 48

#define r1x 144
#define extray 48

int x, y = 0;

char button[4][5] = {
  { '7', '8', '9', '(' , ')'},
  { '4', '5', '6', 'X' , '/'},
  { '1', '2', '3', '-' , '+'},
  { 'C', '0', '.', 'M' , 'B'}
};

void draw()
{
  tft.fillScreen(ILI9341_BLACK);

  tft.drawRoundRect(row1x, extray, boxsize, boxsize, 8, ILI9341_WHITE);
  tft.drawRoundRect(row1x, extray + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
  tft.drawRoundRect(row1x, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
  tft.drawRoundRect(row1x, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_RED);

  for (int b = extray; b <= 192; b += boxsize)
  {
    tft.drawRoundRect  (row1x + boxsize, b, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect  (row1x + boxsize * 3, b, boxsize, boxsize, 8, ILI9341_BLUE);
    tft.drawRoundRect  (row1x + boxsize * 4, b, boxsize, boxsize, 8, ILI9341_BLUE);
  }
  tft.drawRoundRect(row1x + boxsize * 2, extray, boxsize, boxsize, 8, ILI9341_WHITE);
  tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
  tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
  tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_YELLOW);
  tft.drawRoundRect(row1x + boxsize * 5, extray, 320-(row1x + boxsize * 5)-2, boxsize*4, 8, ILI9341_GREEN);

  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 5; i++) {
      tft.setCursor(16 + (boxsize * i), extray + 12 + (boxsize * j));
      tft.setTextSize(3);
      tft.setTextColor(ILI9341_WHITE);
      if (button[j][i]=='B') tft.setCursor(16 + (boxsize * i), extray + 15 + (boxsize * j)), tft.setTextSize(2),tft.println("<-");
      else if (button[j][i]=='M') tft.setCursor(7 + (boxsize * i), extray + 15 + (boxsize * j)), tft.setTextSize(2),tft.println("Mod");
      else tft.println(button[j][i]);
    }
  }
  tft.setCursor(23 + (boxsize * 5), extray + 15 + (boxsize * 1));
  tft.setTextSize(7);
  tft.setTextColor(ILI9341_WHITE);
  tft.println('=');
  tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
  tft.setTextSize(3);
  tft.setCursor(4,12);
}

void setup()
{
  key1 = "";
  key2 = "";
  pinMode(BL_LED, OUTPUT);
  digitalWrite(BL_LED, HIGH);
  Serial.begin(115200);
  Serial.println("Calculator");
  ts.begin();
  tft.begin();
  tft.setRotation(3);
  draw();
  tft.setCursor(4, 12);
}

double str2dble(String & str) {
  return atof( str.c_str() );
}

template<typename T>
struct stack {
    T stk[100];
    int tp=-1;
    void push(T k) {
        stk[++tp]=k;
    }
    bool empty() {
        return tp==-1;
    }
    void pop() {
        if (!(this->empty())) tp--;
    }
    T top() {
        return stk[tp];
    }
    void clear() {
        tp=-1;
    }
};

String screen="";

void ps(String s) {
    tft.print(s);
    screen+=s;
}
void ps(char s) {
    tft.print(s);
    screen+=s;
}



bool isop(double db) {
    return db==-7.122 || db==-8.122 || db==-9.122 || db==-10.122;
}

void loop()
{
  if (ts.touched() && !beentouched)
  {
    if(equal)
    {
      equal = false;
      answer = 0;
      screen="";
      tft.fillRoundRect(0, 0, 320, 48, 8, ILI9341_BLACK);
      tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
      tft.setCursor(4, 12);
    }
    
    /*if(key1.length()+key2.length()+int(operation != ' ') > 16)
    {
      toLong = true;
    }
    */
    TS_Point p = ts.getPoint();     // Read touchscreen
    beentouched = true;

    x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

    lastchar = ' ';
    lastchar = idbutton();

    
    if (lastchar == 'C') {
      equal = false;
      screen="";
      answer = 0;
      tft.fillRoundRect(0, 0, 320, 48, 8, ILI9341_BLACK);
      tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
      tft.setCursor(4, 12);
    } else if (lastchar == '=') {
        calc(screen);
    } else if (lastchar == 'B') {
        screen.remove(screen.length()-1);
        tft.fillRoundRect(0, 0, 320, 48, 8, ILI9341_BLACK);
        tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
        tft.setCursor(4, 12);
        tft.print(screen);
    } else {
        ps(lastchar);
    }
/*
    if ()
    {
      if (key1 != "" && key2 != "")
      {
        equal = true;
        Serial.println("Calculate");
        if (key2.toInt() == 0 && operation == '/')
        {
          tft.setCursor(4, 12);
          tft.fillRoundRect(0, 0, 320, 48, 8, ILI9341_BLACK);
          tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
          tft.print("=");
          tft.setTextColor(ILI9341_RED);
          tft.print("ERROR");
          tft.setTextColor(ILI9341_WHITE);
          key1 = "";
          key2 = "";
          operation = ' ';
        }
        else 
        {
          key1i = 0;
          key2i = 0;
          key1i = str2dble(key1);
          key2i = str2dble(key2);

          if (key1n) key1i*=-1;
          if (key2n) key2i*=-1;
          
          answer = calc(key1i, key2i, operation);
          
          tft.fillRoundRect(0, 0, 320, 48, 8, ILI9341_BLACK);
          tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
          tft.setCursor(4, 12);
          tft.print('=');
          tft.print(answer);
          key1i = 0;
          key2i = 0;
          operation = ' ';
          key1 = "";
          key2 = "";
        }
      }
    }*/
    //wait for release
    while (ts.touched()) {delay(10);};
    beentouched = false;
  }
}

char idbutton()
{
  //Row 1 identification
  if ((x>=0) && (x <= boxsize))
  {
    //    Serial.println("Row 1  ");
    //7
    if (((extray + boxsize) >= y) && (y >= extray))
    {
      tft.drawRoundRect(row1x, extray, boxsize, boxsize, 8, ILI9341_RED);
      delay(100);
      tft.drawRoundRect(row1x, extray, boxsize, boxsize, 8, ILI9341_WHITE);
      return '7';
    }
    //4
    if (((extray + (boxsize * 2)) >= y) && (y >= (extray + boxsize)))
    {
      tft.drawRoundRect(row1x, extray + boxsize, boxsize, boxsize, 8, ILI9341_RED);
      delay(100);
      tft.drawRoundRect(row1x, extray + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
      return '4';
    }
    //1
    if (((extray + (boxsize * 3)) >= y) && (y >= (extray + (boxsize * 2))))
    {
      tft.drawRoundRect(row1x, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_RED);
      delay(100);
      tft.drawRoundRect(row1x, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
      return '1';
    }
    //C
    if (((extray + (boxsize * 4)) >= y) && (y >= (extray + (boxsize * 3))))
    {
      tft.drawRoundRect(row1x, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_WHITE);
      delay(100);
      tft.drawRoundRect(row1x, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_RED);
      return 'C';
    }

  }

  //Row 2 identification
  if ((x>=boxsize) && (x <= (boxsize * 2)))
  {
    //    Serial.println("Row 2  ");
    //8
    if (((extray + boxsize) >= y) && (y >= extray))
    {
      tft.drawRoundRect(row1x + boxsize, extray, boxsize, boxsize, 8, ILI9341_RED);
      delay(100);
      tft.drawRoundRect(row1x + boxsize, extray, boxsize, boxsize, 8, ILI9341_WHITE);
      return '8';
    }
    //5
    if (((extray + (boxsize * 2)) >= y) && (y >= (extray + boxsize)))
    {
      tft.drawRoundRect(row1x + boxsize, extray + boxsize, boxsize, boxsize, 8, ILI9341_RED);
      delay(100);
      tft.drawRoundRect(row1x + boxsize, extray + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
      return '5';
    }
    //2
    if (((extray + (boxsize * 3)) >= y) && (y >= (extray + (boxsize * 2))))
    {
      tft.drawRoundRect(row1x + boxsize, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_RED);
      delay(100);
      tft.drawRoundRect(row1x + boxsize, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
      return '2';
    }
    //0
    if (((extray + (boxsize * 4)) >= y) && (y >= (extray + (boxsize * 3))))
    {
      tft.drawRoundRect(row1x + boxsize, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_RED);
      delay(100);
      tft.drawRoundRect(row1x + boxsize, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_WHITE);
      return '0';
    }
  }

  //Row 3 identification
  if ((x>=(boxsize * 2)) && (x <= (boxsize * 3)))
  {
    //    Serial.println("Row 3  ");
    //9
    if (((extray + boxsize) >= y) && (y >= extray))
    {
      tft.drawRoundRect(row1x + boxsize * 2, extray, boxsize, boxsize, 8, ILI9341_RED);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 2, extray, boxsize, boxsize, 8, ILI9341_WHITE);
      return '9';
    }
    //6
    if (((extray + (boxsize * 2)) >= y) && (y >= (extray + boxsize)))
    {
      tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize, boxsize, boxsize, 8, ILI9341_RED);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
      return '6';
    }
    //3
    if (((extray + (boxsize * 3)) >= y) && (y >= (extray + (boxsize * 2))))
    {
      tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_RED);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
      return '3';
    }
    //=
    if (((extray + (boxsize * 4)) >= y) && (y >= (extray + (boxsize * 3))))
    {
      tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_BLUE);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_YELLOW);
      return '.';
    }

  }

  //Row 4 identification
  if ((x>=(boxsize * 3)) && (x <= (boxsize * 4)))
  {
    //    Serial.println("Row 4  ");
    //+
    if (((extray + boxsize) >= y) && (y >= extray))
    {
      tft.drawRoundRect(row1x + boxsize * 3, extray, boxsize, boxsize, 8, ILI9341_GREEN);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 3, extray, boxsize, boxsize, 8, ILI9341_BLUE);
      return '(';
    }
    //-
    if (((extray + (boxsize * 2)) >= y) && (y >= (extray + boxsize)))
    {
      tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize, boxsize, boxsize, 8, ILI9341_GREEN);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize, boxsize, boxsize, 8, ILI9341_BLUE);
      return 'X';
    }
    //*
    if (((extray + (boxsize * 3)) >= y) && (y >= (extray + (boxsize * 2))))
    {
      tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_GREEN);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_BLUE);
      return '-';
    }
    // /
    if (((extray + (boxsize * 4)) >= y) && (y >= (extray + (boxsize * 3))))
    {
      tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_GREEN);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_BLUE);
      return '%';
    }
  }

  //Row 5 identification
  if ((x>=(boxsize * 4)) && (x <= (boxsize * 5)))
  {
    //    Serial.println("Row 4  ");
    //+
    if (((extray + boxsize) >= y) && (y >= extray))
    {
      tft.drawRoundRect(row1x + boxsize * 4, extray, boxsize, boxsize, 8, ILI9341_GREEN);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 4, extray, boxsize, boxsize, 8, ILI9341_BLUE);
      return ')';
    }
    //-
    if (((extray + (boxsize * 2)) >= y) && (y >= (extray + boxsize)))
    {
      tft.drawRoundRect(row1x + boxsize * 4, extray + boxsize, boxsize, boxsize, 8, ILI9341_GREEN);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 4, extray + boxsize, boxsize, boxsize, 8, ILI9341_BLUE);
      return '/';
    }
    //*
    if (((extray + (boxsize * 3)) >= y) && (y >= (extray + (boxsize * 2))))
    {
      tft.drawRoundRect(row1x + boxsize * 4, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_GREEN);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 4, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_BLUE);
      return '+';
    }
    // /
    if (((extray + (boxsize * 4)) >= y) && (y >= (extray + (boxsize * 3))))
    {
      tft.drawRoundRect(row1x + boxsize * 4, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_GREEN);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 4, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_BLUE);
      return 'B';
    }
  }

  if (x>=(boxsize * 5))
  {
    //    Serial.println("Row 4  ");
    //+
    if (((extray + (boxsize * 4)) >= y) && (y >= extray))
    {
      tft.drawRoundRect(row1x + boxsize * 5, extray, 320-(row1x + boxsize * 5)-2, boxsize*4, 8, ILI9341_BLUE);
      //tft.drawRoundRect(row1x + boxsize * 4, extray, 320-(row1x + boxsize * 4)-5, boxsize*4, 8, ILI9341_BLUE);
      delay(100);
      tft.drawRoundRect(row1x + boxsize * 5, extray, 320-(row1x + boxsize * 5)-2, boxsize*4, 8, ILI9341_GREEN);
      //tft.drawRoundRect(row1x + boxsize * 4, extray, 320-(row1x + boxsize * 4)-5, boxsize*4, 8, ILI9341_GREEN);
      return '=';
    }
  }
}

double op2dble(char op) {
    if (op=='+') return -7.122;
    if (op=='-') return -8.122;
    if (op=='X') return -9.122;
    if (op=='/') return -10.122;
    if (op=='%') return -11.122;
}

char dble2op(double op) {
    if (op==-7.122) return '+';
    if (op==-8.122) return '-';
    if (op==-9.122) return 'X';
    if (op==-10.122) return '/';
    if (op==-11.122) return '%';
    return 'F';
}

int priority(char op) { 
    switch(op) { 
        case '+': case '-': return 1;
        case 'X': case '/': case '%':  return 2;
        default:            return 0;
    } 
} 

String inToPostfix(vector<double> &s) { 
        stack<double> stk;
        String postfix;
    for(int i = 0; i<s.size(); i++) 
        switch(dble2op(s[i])) { 
        case '(':
        stk.push(op2dble('('));
            break; 
        case '+': case '-': case 'X': case '/': case '%':
            while(priority(dble2op(stk.top())) >= priority(s[i])) { 
                postfix+= dble2op(stk.top());
                stk.pop();
            } 
            stk.push(op2dble('('));
            break; 
        case ')': 
            while(dble2op(stk.top()) != '(') {
                postfix+= dble2op(stk.top());
                stk.pop();
            } 
            stk.pop();
            break; 
        default:
            postfix+= s[i];
    }
    while(!stk.empty()) { 
        postfix+= dble2op(stk.top());
        stk.pop();
    }
    return postfix;
} 

double calc(String s) {
    vector<double> vd;
    String buffer="";
    for (int i=0; i<s.length(); i++) {
        if (s[i]=='-' && (i==0 || !(s[i]>='0' && s[i]<='9'))) buffer+='-';
        else if ((s[i]>='0' && s[i]<='9') || s[i]=='.') buffer+=s[i];
        else {
            vd.push_back(str2dble(buffer));
            vd.push_back(op2dble(s[i]));
        }
    }
    vd.push_back(str2dble(buffer));
    String post=inToPostfix(vd);
    Serial.println(post);
}
