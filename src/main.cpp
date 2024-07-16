#include <Arduino.h>
#include <Watchy.h>
#include <Display.h>  

#include "settings.h"
#include "FourDigitDisplay.h"
#include "qrcode.h"

#define W_12_HOUR_MODE  0
#define W_24_HOUR_MODE  1
#define W_DATE_MODE     2
#define W_QR_CODE_MODE  3

#define QR_CODE_VERSION 4
#define QR_CODE_PIXEL_SIZE 5 // 6 pixels would work for this version 4 (33 x 33 pixels) but 5 gives a larger white border
#define QR_CODE_URL_TO_DISPLAY "https://github.com/qcope/WatchySevenSegment"

RTC_DATA_ATTR int watchMode = W_12_HOUR_MODE;
RTC_DATA_ATTR bool qrTimeMode = true; // first button press... actually negates this... so you see URL rather that time first thing

class MyFirstWatchFace : public Watchy{ //inherit and extend Watchy class
    public:
        MyFirstWatchFace(const watchySettings& s) : Watchy(s) {}
        void drawWatchFace(){ //override this method to customize how the watch face looks
          display.fillScreen(GxEPD_WHITE); //clear the screen61
          
          if (watchMode != W_QR_CODE_MODE) { // We are not in QR code mode.... so let's tell the time!
            FourDigitDisplay oneLine = FourDigitDisplay(0, WatchyDisplay::HEIGHT / 4, WatchyDisplay::WIDTH, WatchyDisplay::HEIGHT / 2); //Middle of display
            if (watchMode == W_24_HOUR_MODE) {
              oneLine.setTwentyFourHour(true); //set the time format to 24hr.... default is 12hr
            }
            if (watchMode == W_DATE_MODE) {
              oneLine.setInt(currentTime.Day); //set the date on the display
            } else {
              oneLine.setTime(currentTime.Hour, currentTime.Minute); //set the time on the display
            }
            oneLine.display(&display); //display the time on the screen
          } else {
            // Let's create a QR code with the URL of the github for this code....
            // we'll use the qrcode library to do this
            char stringToCreateCodeFor[90]; // refer to QRCode library, README.md that explains the Data Capabalities of each
                                                    // of the QR code versions.  For example version 8 can handle 122 characters for Alphanumeric
            if (qrTimeMode) {
              sprintf(stringToCreateCodeFor, "%02d:%02d", currentTime.Hour, currentTime.Minute);
            } else {
              sprintf(stringToCreateCodeFor, "%s",QR_CODE_URL_TO_DISPLAY);
            }
            QRCode qrcode;
            int qrcodeversion=QR_CODE_VERSION;
            uint8_t qrcodeBytes[qrcode_getBufferSize(qrcodeversion)];
            int pixelSize = QR_CODE_PIXEL_SIZE;
            qrcode_initText(&qrcode, qrcodeBytes, qrcodeversion, ECC_MEDIUM, stringToCreateCodeFor);
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

