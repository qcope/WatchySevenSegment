#include <Arduino.h>
#include <Watchy.h>
#include <Display.h>  

#include "settings.h"
#include "FourDigitDisplay.h"


class MyFirstWatchFace : public Watchy{ //inherit and extend Watchy class
    public:
        MyFirstWatchFace(const watchySettings& s) : Watchy(s) {}
        void drawWatchFace(){ //override this method to customize how the watch face looks
          display.fillScreen(GxEPD_WHITE); //clear the screen
          
          FourDigitDisplay oneLine = FourDigitDisplay(0, WatchyDisplay::HEIGHT / 4, WatchyDisplay::WIDTH, WatchyDisplay::HEIGHT / 2); //Top of display
          oneLine.setTime(currentTime.Hour, currentTime.Minute); //set the time on the display
          oneLine.display(&display); //display the time on the screen
        }
};

MyFirstWatchFace m(settings); 

void setup(){
  m.init(); 
}

void loop(){  // don't think we'll get here
}

