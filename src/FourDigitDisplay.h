#ifndef FOURDIGITDISPLAY_H
#define FOURDIGITDISPLAY_H

#undef             DEBUG_FOURDIGITDISPLAY

#include <Watchy.h>
#include "SevenSegmentDigit.h"

#define COLON_WIDTH 20  // SPACE FOR COLON
#define COLON_RADIUS 5

class FourDigitDisplay {
    private:
        int x, y, width, height, digit_width;
        bool twenty_four_hour = false;
        bool colon = true;
    
        SevenSegmentDigit digits[4]={
            SevenSegmentDigit(0,0,0,0),
            SevenSegmentDigit(0,0,0,0),
            SevenSegmentDigit(0,0,0,0),
            SevenSegmentDigit(0,0,0,0)
        };

    public:
        FourDigitDisplay(int x, int y, int width, int height);
        void clearDisplay();
        void setDigit(int digit, int number);
        void setTime(int hour, int minute);
        void setInt(int number);
        void setTwentyFourHour(bool state);
        // void setDecimalPoint(int digit, bool state);
        void display(GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *watchDisplay);
};
#endif // FOURDIGITDISPLAY_H
