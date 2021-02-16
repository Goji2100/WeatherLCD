#include <stdio.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "config.h"

#define WDCNT     48

#if defined(TESTDATA)
#include "wdata.h"
#else
int16_t wdhour[WDCNT], wdtemp[WDCNT], wdpopr[WDCNT];
#endif

#define TIME_ZONE (9 * (60 * 60))

#if (1)
#define LAT      "35.535693"    // Latitude   Kawasaki
#define LAN     "139.726133"    // longitude

#elif (0)
#define LAT      "43.025883"    // Latitude   Wakkanai
#define LAN     "141.976180"    // longitude

#else
#define LAT      "26.421949"    // Latitude   Okinawa
#define LAN     "127.809439"    // longitude
#endif

#define OWURL   "http://api.openweathermap.org/data/2.5/onecall"
#define OPTION  "&lang=en&units=metric&exclude=current,minutely,daily,alerts"

/*  APPID define in "config.h"
#define APPID   "Your APPID from openWeather"
*/

#define openWeather OWURL "?lat=" LAT "&lon=" LAN OPTION "&APPID=" APPID

#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

#define __RGB(Rr,Gg,Bb) ((Rr<<11)|(Gg<<5)|Bb)
#define TFT_RED2    __RGB(30,  0,  0)
#define TFT_RED3    __RGB(16,  0,  0)
#define TFT_RED4    __RGB(30, 36, 17)
#define TFT_BLUE2   __RGB( 0,  0, 15)
#define TFT_BLUE3   __RGB( 0,  0,  8)
#define TFT_BLUE4   __RGB(15, 32, 21)
#define TFT_CYAN2   __RGB( 0, 40, 20)
#define TFT_WHITE2  __RGB(28, 56, 28)
#define TFT_WHITE3  __RGB(15, 30, 15)
#define TFT_WHITE4  __RGB(10, 20, 10)

#define GFXFF 1
#define FSB12 &FreeSerifBold12pt7b

struct tm timeInfo;
uint8_t sw18;

void setup() {
  Serial.begin(115200);
  Serial.println("\n");
  pinMode(18, INPUT_PULLUP);
  sw18 = digitalRead(18);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  WiFi.begin(SSID, PSWD);
  Serial.println("Connected to the WiFi network - " SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  configTime(TIME_ZONE, 0, "ntp.nict.jp");
  init_Clock();
}

void loop() {
  static int ss = -1;

  getLocalTime(&timeInfo);
  if (ss != timeInfo.tm_sec) {
    ss = timeInfo.tm_sec;
    disp_Clock();
#if defined(TESTDATA)
    if (ss % 5 == 0) {
      if (1) {
        do delay(10); while (sw18 == digitalRead(18));
        do delay(10); while (sw18 != digitalRead(18));
        sw18 = digitalRead(18);
        do delay(10); while (sw18 == digitalRead(18));
      }
      disp_Weather();
      if (1) {
        int16_t h = wdhour[0], t = wdtemp[0], p = wdpopr[0];
        for (int i = 0; (i < (WDCNT - 1)); i++) {
          wdhour[i] = wdhour[i + 1];
          wdtemp[i] = wdtemp[i + 1];
          wdpopr[i] = wdpopr[i + 1];
        }
        wdhour[WDCNT - 1] = h, wdtemp[WDCNT - 1] = t, wdpopr[WDCNT - 1] = p;
      }
#else
    if ( (ss == 0) && ((timeInfo.tm_min % 5) == 0) ) {  // Interval >= 2
      disp_Weather();
#endif
    }
  }
  delay(50);
}
