#include <Adafruit_ILI9341.h>

class ButtonCoordinate {
public:
    ButtonCoordinate() {
        this->x = 0;
        this->y = 0;
        this->width = 0;
        this->height = 0;
    }

    ButtonCoordinate(int x, int y, int width, int height) {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
    }

    bool isPressed(int input_X, int input_Y) {
        return (input_X > x && input_X < x + width && input_Y > y && input_Y < y + height);
    }

    void fillAndDraw(Adafruit_ILI9341& tft, int color1, int color2) {
        tft.fillRect(x, y, width, height, color1);
        tft.drawRect(x, y, width, height, color2);
    }

    int x;
    int y;
    int width;
    int height;

};