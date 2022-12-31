#include <Adafruit_GFX.h>  // Core graphics library
#include <MCUFRIEND_kbv.h> // Hardware-specific library
MCUFRIEND_kbv tft;

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>

#include <FreeDefaultFonts.h>

#define BLACK 0x0000
#define RED 0xF800
#define GREEN 0x07E0
#define WHITE 0xFFFF
#define GREY 0x8410

void setup(void)
{
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  Serial.println("Example: Font_simple");
  Serial.print("found ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3)
    ID = 0x9481; // force ID if write-only display
  tft.begin(ID);
  initializeSetup();
}

String str = "";
int i = 0;
void loop(void)
{
  int hh = (i / 3600) % 24;
  int mm = (i / 60) % 60;
  int ss = i % 60;
  char o[8];
  sprintf(o, "%2d:%02d:%02d", hh, mm, ss);
  tft.setCursor(30, 140);

  tft.print(o);

  i++;
  delay(1000);
}

void initializeSetup()
{
  tft.setRotation(181);
  tft.fillScreen(BLACK);
  tft.setTextColor(RED, BLACK);
  tft.setTextSize(8);
}
