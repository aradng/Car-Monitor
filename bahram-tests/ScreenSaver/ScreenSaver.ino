#include <Adafruit_GFX.h>   // Core graphics library
#include <MCUFRIEND_kbv.h>  // Hardware-specific library
MCUFRIEND_kbv tft;

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>

#include <FreeDefaultFonts.h>

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

void setup(void) {
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3)
    ID = 0x9481;  // force ID if write-only display
  tft.begin(ID);
  tft.setRotation(3);
  tft.fillScreen(BLACK);
}

int x = 0;
int y = 0;
bool xPluse = true;
bool yPluse = true;

void loop(void) {
  draw(x, y, BLACK);
  if (x == 460) {
    xPluse = false;
  } else if (x == 0) {
    xPluse = true;
  };

  if (y == 300) {
    yPluse = false;
  } else if (y == 0) {
    yPluse = true;
  };

  if (xPluse) {
    x++;
  } else {
    x--;
  };

  if (yPluse) {
    y++;
  } else {
    y--;
  };
  draw(x, y, RED);
  delay(10);
}

void draw(int x, int y, const uint16_t color) {
  tft.fillRect(x, y, 20, 20, color);
}
