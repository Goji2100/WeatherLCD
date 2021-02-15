/*
 TFT_Clock Copyright (c) 2020 Bodmer
 Based on a sketch by Gilchrist 6/2/2014 1.0

 An example analogue clock using a TFT LCD screen to show the time
 use of some of the drawing commands with the library.

 Make sure all the display driver and pin comnenctions are correct by
 editting the User_Setup.h file in the TFT_eSPI library folder.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 ######################################################################### 

#define ST7789_DRIVER

#if defined(D_duino_32_XS)
#define DEV_NAME "ST7789_DRIVER"
#define TFT_WIDTH  240
#define TFT_HEIGHT 240
#define TFT_SCLK 27
#define TFT_MOSI 26
#define TFT_DC   23  // Data Command control pin
#define TFT_RST  32  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
#define TFT_BL   22  // LED back-light
#endif 

*/

#define DTBX    108   // x : Coordinates of the calendar display area
#define DTBY      4   // y : ..
#define DTBW    130   // w : Width
#define DTBH     86   // h : Height

#define CLCX     50   // x : Coordinates of the center of an analog clock
#define CLCY     50   // y : ..
#define CLCR     40   // r : Radius of an analog clock

void init_Clock(void) {
  // Analog clock
  tft.drawCircle(CLCX       , CLCY       , CLCR    , TFT_BLUE2);
  tft.drawCircle(CLCX       , CLCY       , CLCR - 1, TFT_BLUE2);
  tft.fillCircle(CLCX       , CLCY - CLCR,        2, TFT_WHITE);  // 00:00
  tft.fillCircle(CLCX + CLCR, CLCY       ,        2, TFT_CYAN);   // 03:00
  tft.fillCircle(CLCX       , CLCY + CLCR,        2, TFT_CYAN);   // 06:00
  tft.fillCircle(CLCX - CLCR, CLCY       ,        2, TFT_CYAN);   // 09:00
}

void disp_Clock() {
  static int dd = 0;
  char edits[16];

  // Calendar - 2020/11/15
  if (dd != timeInfo.tm_mday) {       // Did it change the date?
    dd = timeInfo.tm_mday;
    tft.fillRoundRect(DTBX,  DTBY + 4, DTBW, DTBH, 3, TFT_BLUE2);
    tft.setFreeFont(FSB12);
    tft.setTextColor(TFT_WHITE3, TFT_BLUE2);
    tft.setTextDatum(TL_DATUM);
    sprintf(edits, "%04d/%02d/%02d", timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday);
    tft.drawString(edits, DTBX + 8, DTBY + 20, GFXFF);

    for (int i = 0; (i < 7); i++) {               // day of the week
      uint16_t bc = ((i >= 5) ? TFT_RED3 : TFT_WHITE4);
      if (i == ((timeInfo.tm_wday + 6) % 7)) {
        bc = ((i >= 5) ? TFT_RED2 : TFT_WHITE2);  // Saturday / Sunday
      }
      tft.fillRect(DTBX + 9 + (16 * i), DTBY + 44, 14, 6, bc);
    }
  }

  // Digital clock - 12:34
  if (timeInfo.tm_sec == 0) {       // Did the time change?
    tft.setFreeFont(FSB12);
    tft.setTextColor(TFT_WHITE2, TFT_BLUE2);
    tft.setTextDatum(TL_DATUM);
    sprintf(edits, "%02d:%02d ", timeInfo.tm_hour, timeInfo.tm_min);
    tft.drawString(edits, DTBX + 62, DTBY + 58, GFXFF);
  }

   // Analog clock
  static int oxy[6] = { 0 };        // Erase Old Time
  if (oxy[0] != 0) {
    tft.drawLine(CLCX, CLCY, oxy[0], oxy[1], TFT_BLACK);  // HH
    tft.drawLine(CLCX, CLCY, oxy[2], oxy[3], TFT_BLACK);  // MM
    tft.drawLine(CLCX, CLCY, oxy[4], oxy[5], TFT_BLACK);  // SS
  }

  float sd = (timeInfo.tm_sec  *  6);
  float md = (timeInfo.tm_min  *  6) + (sd * 0.01666667);
  float hd = (timeInfo.tm_hour * 30) + (md * 0.08333333);

  int hx = (cos((hd - 90) * 0.0174532925)) * (CLCR - 16) + CLCX;
  int hy = (sin((hd - 90) * 0.0174532925)) * (CLCR - 16) + CLCY;
  int mx = (cos((md - 90) * 0.0174532925)) * (CLCR - 10) + CLCX;
  int my = (sin((md - 90) * 0.0174532925)) * (CLCR - 10) + CLCY;
  int sx = (cos((sd - 90) * 0.0174532925)) * (CLCR - 10) + CLCX;
  int sy = (sin((sd - 90) * 0.0174532925)) * (CLCR - 10) + CLCY;

  tft.drawLine(CLCX, CLCY, oxy[0] = hx, oxy[1] = hy, TFT_CYAN);
  tft.drawLine(CLCX, CLCY, oxy[2] = mx, oxy[3] = my, TFT_WHITE);
  tft.drawLine(CLCX, CLCY, oxy[4] = sx, oxy[5] = sy, TFT_RED);
  tft.fillCircle(CLCX, CLCY, 2, TFT_RED);
}
