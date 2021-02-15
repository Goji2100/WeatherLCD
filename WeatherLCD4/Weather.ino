#include <TimeLib.h>

#if defined(ARDUINO_M5Stack_Core_ESP32)
#define WCDSPCNT 32
#define WCBX      0
#define WCBY     98
#define WCBW    320
#define WCBH    110

#else
#define WCDSPCNT 24
#define WCBX      0
#define WCBY     98
#define WCBW    240
#define WCBH    113
#endif

void get_Weather() {
  HTTPClient http;

  http.begin(openWeather);
  int httpRet = http.GET();

  if (httpRet > 0) {
    String jsonString = http.getString();
    if (0) Serial.println(jsonString);

    DynamicJsonDocument weatherdata(40000);
    deserializeJson(weatherdata, jsonString);

    for (int i = 0; (i < WDCNT); i++) {
      uint32_t wdt = weatherdata["hourly"][i]["dt"].as<int>() + TIME_ZONE;
      tmElements_t wt;
      breakTime(wdt, wt);
      wdhour[i] = wt.Hour;
      wdtemp[i] = weatherdata["hourly"][i]["temp"].as<float>() *  10;
      wdpopr[i] = weatherdata["hourly"][i]["pop" ].as<float>() * 100;
      wdhour[i] = ((wdhour[i] >  23) ?  23 : ((wdhour[i] <    0) ?    0 : wdhour[i]));  //   0 ..   23
      wdtemp[i] = ((wdtemp[i] > 500) ? 500 : ((wdtemp[i] < -500) ? -500 : wdtemp[i]));  // -50 ..  +50
      wdpopr[i] = ((wdpopr[i] > 100) ? 100 : ((wdtemp[i] <    0) ?    0 : wdpopr[i]));  //   0 ..  100
      if (1) {
        Serial.printf("%4d/%02d/%02d %02d - ", wt.Year + 1970, wt.Month, wt.Day, wt.Hour);
        Serial.printf("%3d - %3d\n", wdtemp[i], wdpopr[i]);
      }
    }
  } else Serial.printf("Error on HTTP request (%d)", httpRet);
  http.end();
}

void DrawWline(int px0, int py0, int px1, int py1, uint16_t pc) {
  tft.drawLine(px0, py0,     px1, py1,     pc);
  tft.drawLine(px0, py0 - 1, px1, py1 - 1, pc);
}

static int tstep[] = { 10, 20, 50,100,200 };

void disp_Weather() {
  char edits[16];

#if !defined(TESTDATA)
  get_Weather();
#endif

  tft.fillRect(WCBX, WCBY    , WCBW, WCBH    , TFT_BLUE3 );
  tft.drawRect(WCBX, WCBY - 1, WCBW, WCBH + 2, TFT_WHITE4);

  // Check max / min Temperature
  int tmin = 500, tmax = -500, tunit, tbase;
  for (int i = 1; (i < WCDSPCNT); i++) {
    if (wdtemp[i] < tmin) tmin = wdtemp[i];
    if (wdtemp[i] > tmax) tmax = wdtemp[i];
  }
  Serial.printf("max=%d,min=%d ",tmax, tmin);

  // Scale calculation
  tmax += 500; tmin += 500;
  for (int i = 0; (i < 5); tunit = tstep[++i]) {
    tunit = tstep[i];
    tmax  = ((tmax + (tunit - 1)) / tunit) * tunit;
    tmin  = ((tmin              ) / tunit) * tunit;
    if ( ((tmax - tmin + (tunit - 1)) / tunit) < 10 ) break;
  }
  tbase = tmin - ( ( ( 10 - ( (tmax - tmin) / tunit ) ) / 2 ) * tunit );
  if ((tbase > 500) && (tunit <= 20) && (((tbase + (tunit * 5)) / 10) % 2) ) tbase += 10;
  tmax -= 500, tmin -= 500, tbase -= 500;
  Serial.printf("max=%d,min=%d,base=%d unit=%d\n", tmax, tmin, tbase, tunit);

  // H line each 10 line
  for (int i = 1; (i <= 10); i++) {
    int bc = (i == 5) ? TFT_CYAN2 : ( (i % 2 == 0) ? TFT_WHITE3 : TFT_WHITE4 );
    if ((tbase + (tunit * i)) == 0) bc = TFT_WHITE2;
    tft.drawLine(WCBX, WCBY + WCBH - 1 - (i * 10), WCBX + WCBW - 1, WCBY + WCBH - 1 - (i * 10), bc);
  }

  // V line each 3h
  for (int i = 1; (i < (WCDSPCNT - 1)); i++) {
    if (((wdhour[0] + i) % 3) == 0) {
      tft.drawLine(WCBX + (i * 10), WCBY + 12, WCBX + (i * 10), WCBY + WCBH - 1, TFT_WHITE4);
      sprintf(edits, "%d", wdhour[i]);
      drawFont8x12(edits, WCBX + (i * 10) - ((strlen(edits) > 1) ? 7 : 3), WCBY + 1, TFT_WHITE3, TFT_BLUE3);
    }
  }

   // Plot Chance of rain and // Temperature
   for (int i = 1; (i < (WCDSPCNT - 1)); i++) {
    int x0 = WCBX + ((i    ) * 10);
    int x1 = WCBX + ((i + 1) * 10);

    // Chance of rain
    int y0 = (WCBY + WCBH - 2) - wdpopr[i    ];
    int y1 = (WCBY + WCBH - 2) - wdpopr[i + 1];
    DrawWline(x0, y0, x1, y1, TFT_CYAN);

    // Temperature
    y0 = (WCBY + WCBH - 1 - 100) + (100 - (wdtemp[i    ] - tbase) / (tunit / 10));
    y1 = (WCBY + WCBH - 1 - 100) + (100 - (wdtemp[i + 1] - tbase) / (tunit / 10));
    DrawWline(x0, y0, x1, y1, TFT_RED4);
    Serial.printf("%02d - %4d - %3d\n", wdhour[i], wdtemp[i], wdpopr[i]);
  }

  // Draw scale
  int tzero = ( (tbase < 0) && ((tbase % tunit) == 0) && ((tbase / tunit) % 2 == 0) ) ? 1 : 0;
  for (int i = 1; (i < 10); i++) {  // Temperature
    if ( ((tzero == 1) && (i % 2 == 0)) || ((tzero == 0) && (i % 2 == 1)) ) {
      int t = ((tbase + (tunit * (10 - i))) / 10);
      sprintf(edits, "%2d", abs(t));
      drawFont8x12(edits, (WCBX + 1), (WCBY + WCBH - 100 + (i * 10) - 5), ((t > 0) ? TFT_RED4 : TFT_WHITE2), TFT_BLUE3);
    }

    if (i % 2 == 1) {               // Chance of rain
      sprintf(edits, "%2d", (100 - (i * 10)));
      drawFont8x12(edits, (WCBX + WCBW - 17), (WCBY + WCBH - 100 + (i * 10) - 5), TFT_CYAN2, TFT_BLUE3);
    }
  }
}
