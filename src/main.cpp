#include <Arduino.h>
#include <Watchy.h>
#include <Display.h>  

#include "settings.h"
#include "FourDigitDisplay.h"
#include "qrcode.h"

#define W_12_HOUR_MODE  0
#define W_24_HOUR_MODE  1
#define W_DATE_MODE     2
#define W_ANALOG_MODE   3
#define W_QR_CODE_MODE  4

#define QR_CODE_VERSION 4
#define QR_CODE_PIXEL_SIZE 5 // 6 pixels would work for this version 4 (33 x 33 pixels) but 5 gives a larger white border
#define QR_CODE_URL_TO_DISPLAY "https://github.com/qcope/WatchySevenSegment"

#define SMALL_TIME_BORDER 3
#define LARGE_TICK_ANGLE 2 
#define ANALOGUE_RADIUS 90
#define HOUR_HAND_LENGTH 60
#define HOUR_HAND_WIDTH 10
#define MINUTE_HAND_LENGTH 85
#define MINUTE_HAND_WIDTH 8
#define CENTRE_CIRCLE_RADIUS 10

RTC_DATA_ATTR int watchMode = W_12_HOUR_MODE;
RTC_DATA_ATTR bool qrTimeMode = true; // first button press... actually negates this... so you see URL rather that time first thing

class AnaloguePointer {
  private:
    int xCentre, yCentre, length, width;
    int x1,y1,x2,y2,x3,y3,x4,y4;
    GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display;
  public:
    AnaloguePointer(int xCentre, int yCentre, int length, int width, GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display) {
      this->xCentre = xCentre;
      this->yCentre = yCentre;
      this->length = length;
      this->width = width;
      this->display = display;

      // lets imagine the pointer vertical and pointing up
      x1 = xCentre - width / 2;
      x2 = xCentre + width / 2;
      x3 = x2;
      x4 = x1;
      y1 = yCentre - length;
      y2 = y1;
      y3 = yCentre;
      y4 = yCentre;
    }
    void rotate(int angle) {
      if ( (angle <= 0) || (angle >= 360) ) {
        return;
      }
      // Now rotate these four points about the centre... by the angle
      int x1New = xCentre + (x1 - xCentre) * cos(angle * PI / 180) - (y1 - yCentre) * sin(angle * PI / 180);
      int y1New = yCentre + (x1 - xCentre) * sin(angle * PI / 180) + (y1 - yCentre) * cos(angle * PI / 180);
      int x2New = xCentre + (x2 - xCentre) * cos(angle * PI / 180) - (y2 - yCentre) * sin(angle * PI / 180);
      int y2New = yCentre + (x2 - xCentre) * sin(angle * PI / 180) + (y2 - yCentre) * cos(angle * PI / 180);
      int x3New = xCentre + (x3 - xCentre) * cos(angle * PI / 180) - (y3 - yCentre) * sin(angle * PI / 180);
      int y3New = yCentre + (x3 - xCentre) * sin(angle * PI / 180) + (y3 - yCentre) * cos(angle * PI / 180);
      int x4New = xCentre + (x4 - xCentre) * cos(angle * PI / 180) - (y4 - yCentre) * sin(angle * PI / 180);
      int y4New = yCentre + (x4 - xCentre) * sin(angle * PI / 180) + (y4 - yCentre) * cos(angle * PI / 180);
      x1 = x1New;
      y1 = y1New;
      x2 = x2New;
      y2 = y2New;
      x3 = x3New;
      y3 = y3New;
      x4 = x4New;
      y4 = y4New;    
    }
    void draw() {
      display->fillTriangle(x1, y1, x2, y2, x3, y3, GxEPD_BLACK);
      display->fillTriangle(x4, y4, x3, y3, x1, y1, GxEPD_BLACK);
    }
};


class MyFirstWatchFace : public Watchy{ //inherit and extend Watchy class
    public:
        MyFirstWatchFace(const watchySettings& s) : Watchy(s) {}
        void drawWatchFace(){ //override this method to customize how the watch face looks

          display.fillScreen(GxEPD_WHITE); //clear the screen first of all
          // Do some housework with the step counter... reset it at midnight
          if (currentTime.Hour == 0 && currentTime.Minute == 0){
            sensor.resetStepCounter();
          }
          // Now if we are going to display a QR code.... let's do that first
          if (watchMode == W_QR_CODE_MODE) {
            char stringToCreateCodeFor[90];         // refer to QRCode library, README.md that explains the Data Capabalities of each
                                                    // of the QR code versions.  For example version 8 can handle 122 characters for Alphanumeric
            int errorCorrection=ECC_MEDIUM; // default... we may go higher... for a short string
            if (qrTimeMode) {
              sprintf(stringToCreateCodeFor, "T: %02d:%02d S: %d", currentTime.Hour, currentTime.Minute, sensor.getCounter());
              errorCorrection=ECC_HIGH; // we can afford to go higher for the time
            } else {
              sprintf(stringToCreateCodeFor, "%s",QR_CODE_URL_TO_DISPLAY);
            }
            QRCode qrcode;
            int qrcodeversion=QR_CODE_VERSION;
            
            uint8_t qrcodeBytes[qrcode_getBufferSize(qrcodeversion)];
            int pixelSize = QR_CODE_PIXEL_SIZE;
            qrcode_initText(&qrcode, qrcodeBytes, qrcodeversion, errorCorrection, stringToCreateCodeFor);
            int xoffset = (display.width() - qrcode.size * pixelSize) / 2;
            int yoffset = (display.height() - qrcode.size * pixelSize) / 2;

            for (uint8_t y = 0; y < qrcode.size; y++) {
                for (uint8_t x = 0; x < qrcode.size; x++) {
                    if(qrcode_getModule(&qrcode, x, y)){
                      display.fillRect(xoffset + x * pixelSize, yoffset + y * pixelSize, pixelSize, pixelSize, GxEPD_BLACK);
                    } else {
                      display.fillRect(xoffset + x * pixelSize, yoffset + y * pixelSize, pixelSize, pixelSize, GxEPD_WHITE);
                    }
                }
            }
          }

          if ( (watchMode == W_12_HOUR_MODE) || (watchMode == W_24_HOUR_MODE) || (watchMode == W_DATE_MODE) )
          {
            FourDigitDisplay oneLine = FourDigitDisplay(0,display.height()/4,display.width(),display.height()/2 );
            if (watchMode == W_24_HOUR_MODE) {
              oneLine.setTwentyFourHour(true); //set the time format to 24hr.... default is 12hr
            }
            if (watchMode == W_DATE_MODE) {
              oneLine.setInt(currentTime.Day); //set the date on the display
            } else {
              oneLine.setTime(currentTime.Hour, currentTime.Minute); //set the time on the display
            }
            oneLine.display(&display); //display the time on the screen
          }

          if (watchMode == W_ANALOG_MODE) {
            int xCentre = display.width()/2;
            int yCentre = display.height()/2;
            // Let's draw ticks for each minute
            int tickOffset;
            for(int i=0; i<=45; i+=6) {
                tickOffset = 100 * tan(i * PI / 180); // Who on earth wants RADIANS !!!!
                display.drawLine(xCentre, yCentre, xCentre + tickOffset, 0, GxEPD_BLACK);
                display.drawLine(xCentre, yCentre, display.width(), yCentre - tickOffset, GxEPD_BLACK);
                display.drawLine(xCentre, yCentre, display.width(), yCentre + tickOffset, GxEPD_BLACK);
                display.drawLine(xCentre, yCentre, xCentre + tickOffset, display.height(), GxEPD_BLACK);
                display.drawLine(xCentre, yCentre, xCentre - tickOffset, display.height(), GxEPD_BLACK);
                display.drawLine(xCentre, yCentre, 0, yCentre + tickOffset, GxEPD_BLACK);
                display.drawLine(xCentre, yCentre, 0, yCentre - tickOffset, GxEPD_BLACK);
                display.drawLine(xCentre, yCentre, xCentre - tickOffset, 0, GxEPD_BLACK); 
            }
            // Now draw some triangles for bigger marks for 12,3,6,9 HOUR MARKS 
            tickOffset = 100 * tan(LARGE_TICK_ANGLE * PI / 180);
            display.fillTriangle(xCentre, yCentre,xCentre + tickOffset, 0, xCentre - tickOffset, 0, GxEPD_BLACK);
            display.fillTriangle(xCentre, yCentre,xCentre + tickOffset, display.height(), xCentre - tickOffset, display.height(), GxEPD_BLACK);
            display.fillTriangle(xCentre, yCentre,0, yCentre + tickOffset, 0, yCentre - tickOffset, GxEPD_BLACK);
            display.fillTriangle(xCentre, yCentre,display.width(), yCentre + tickOffset, display.width(), yCentre - tickOffset, GxEPD_BLACK);
            // Now draw the triangles for 5, 10, 20, 25, 35, 40, 50, 55 minute marks
            int tickOffset1 = 100 * tan( ( 30 - LARGE_TICK_ANGLE /2) * PI /180);
            int tickOffset2 = 100 * tan( ( 30 + LARGE_TICK_ANGLE /2) * PI /180);
            display.fillTriangle(xCentre,yCentre,xCentre + tickOffset1, 0, xCentre + tickOffset2, 0, GxEPD_BLACK);
            display.fillTriangle(xCentre,yCentre,display.width(), yCentre - tickOffset1, display.width(), yCentre - tickOffset2, GxEPD_BLACK);
            display.fillTriangle(xCentre,yCentre,display.width(), yCentre + tickOffset1, display.width(), yCentre + tickOffset2, GxEPD_BLACK);
            display.fillTriangle(xCentre,yCentre,xCentre + tickOffset1, display.height(), xCentre + tickOffset2, display.height(), GxEPD_BLACK);
            display.fillTriangle(xCentre,yCentre,xCentre - tickOffset1, display.height(), xCentre - tickOffset2, display.height(), GxEPD_BLACK);
            display.fillTriangle(xCentre,yCentre,0, yCentre + tickOffset1, 0, yCentre + tickOffset2, GxEPD_BLACK);
            display.fillTriangle(xCentre,yCentre,0, yCentre - tickOffset1, 0, yCentre - tickOffset2, GxEPD_BLACK);
            display.fillTriangle(xCentre,yCentre,xCentre - tickOffset1, 0, xCentre - tickOffset2, 0, GxEPD_BLACK);
            // Now white space in the middle
            display.fillCircle(xCentre, yCentre, ANALOGUE_RADIUS, GxEPD_WHITE);

            // Now let's work on the hands

            AnaloguePointer hourHand = AnaloguePointer(xCentre, yCentre, HOUR_HAND_LENGTH, HOUR_HAND_WIDTH, &display);
            AnaloguePointer minuteHand = AnaloguePointer(xCentre, yCentre, MINUTE_HAND_LENGTH, MINUTE_HAND_WIDTH, &display);

            int hourAngle = (currentTime.Hour % 12) * 30 + currentTime.Minute / 2;
            int minuteAngle = currentTime.Minute * 6;
            
            hourHand.rotate(hourAngle);
            minuteHand.rotate(minuteAngle);

            hourHand.draw();
            minuteHand.draw();
            display.fillCircle(xCentre, yCentre, CENTRE_CIRCLE_RADIUS, GxEPD_BLACK);

           }


          if ( (watchMode == W_QR_CODE_MODE) && qrTimeMode ) {
            int xPos =  60, yPos = 85, width = 80, height = 30;
            display.fillRect(xPos,yPos,width,height,GxEPD_WHITE);
            display.drawRect(xPos,yPos,width,height,GxEPD_BLACK);
            FourDigitDisplay oneLine = FourDigitDisplay(xPos+SMALL_TIME_BORDER,yPos+SMALL_TIME_BORDER,width - 2 * SMALL_TIME_BORDER,height - 2 * SMALL_TIME_BORDER);
            oneLine.setTwentyFourHour(true); //set the time format to 24hr.... default is 12hr
            oneLine.setTime(currentTime.Hour, currentTime.Minute); //set the time on the display
            oneLine.display(&display); //display the time on the screen
          }
        }
        
        void handleButtonPress(){ 
          if (guiState == WATCHFACE_STATE) {
              uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

              if (wakeupBit & UP_BTN_MASK) {
                  watchMode = W_QR_CODE_MODE;
                  qrTimeMode = false;
                  RTC.read(currentTime);
                  showWatchFace(true);
              }
              if (wakeupBit & DOWN_BTN_MASK) {
                  watchMode += 1;
                  qrTimeMode = true;
                  if (watchMode > W_QR_CODE_MODE) {
                    watchMode = W_12_HOUR_MODE;
                  }
                  RTC.read(currentTime);
                  showWatchFace(true);
              }
              if (wakeupBit & MENU_BTN_MASK) {
                  Watchy::handleButtonPress();
                  return;
              }
          } else {
              Watchy::handleButtonPress();
          }
        }
};

MyFirstWatchFace m(settings); 

void setup(){

  /*
  Serial.begin(115200); //initialize serial monitor
  while (!Serial); //wait for serial monitor to open

  delay(1000); //wait for everything else to start up
  Serial.println("Starting Watchy!"); //print to serial monitor
  delay(1000);
  */

  m.init(); 
}

void loop(){  // don't think we'll get here
}

