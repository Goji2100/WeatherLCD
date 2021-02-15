#include "font8x12.h"   // ICONs & Fonts

#define BLACKx  __RGB( 1,  1,  1) // transparent color

void drawFont8x12(char *s, int16_t pX, uint16_t pY, uint16_t fccode, uint16_t bccode) {
  while (*s) {
    tft_drawFont(pX, pY, 8, 2, 11, &ASCIIs[*(uint8_t*)s++][0], fccode, bccode);
    pX += 8;
  }
}


uint16_t tft_getFont(uint8_t *font, int16_t width, int16_t line) {
  if (width <= 8) {
    return (*(font + line) << 8);
  } else {
    if (width <= 12) {
      uint16_t n = line + (line >> 1);    // n = line * 1.5
      if (line & 1)
        return ((*(font + n) << 12) | (*(font + n + 1) << 4));
      else
        return ((*(font + n) <<  8) | (*(font + n + 1) & 0xF0));
    } else {
      return (*(uint16_t *)((uint8_t *)font + ((width / 8) * line)));
    }
  }
}

void tft_drawFont(int16_t pX, int16_t pY, int16_t width, int16_t topl, int16_t line, uint8_t *font, uint16_t fccode, uint16_t bccode) {
  for (int i = 0; (i < line); i++) {
    uint16_t p = tft_getFont(font, width, i + topl);
    for (int j = 0, k = 0x8000; j < width; j++, k >>= 1) {
      if (p & k) {
        tft.drawPixel(pX + j, pY + i, fccode);
      } else if (bccode != BLACKx) {
        tft.drawPixel(pX + j, pY + i, bccode);
      }
    }
  }
}
