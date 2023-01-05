
#include <SFE_BMP180.h>
#include <Wire.h>
#include <vector>

#include "car-lcd.c"

#include <Adafruit_GFX.h>   // Core graphics library
#include <MCUFRIEND_kbv.h>  // Hardware-specific library
MCUFRIEND_kbv tft;

SFE_BMP180 pressure;

#define BLACK 0x0000
#define RED 0xE022
#define GREEN 0x5DE9
#define YELLOW 0xFDE0
#define WHITE 0xFFFF
#define GREY 0x8410


void setup() {
  Serial.begin(9600);
  uint16_t ID = tft.readID();

  if (ID == 0xD3D3)
    ID = 0x9481;  // force ID if write-only display
  tft.begin(ID);

  if (pressure.begin())
    Serial.println("BMP180 init success");
  else {

    Serial.println("BMP180 init fail\n\n");
    while (1)
      ;
  }

  initializeSetup();
}

int ecoY = 270;
char status;
double T, P, p0, p1, a;
double H = 0;
std::vector<double> averageHeight;
std::vector<double> averageTemp;


void initializeSetup() {
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  tft.drawRGBBitmap(157, 43, vwLogo, 153, 145);
  tft.drawRGBBitmap(145, 225, name, 189, 15);

  delay(3000);

  tft.drawRGBBitmap(0, 0, car_lcd, 480, 320);

  tft.setTextColor(WHITE, BLACK);

  for (int i = 0; i < 5; i++) {
    getSensoreData();
    averageHeight.emplace_back(H);
    averageTemp.emplace_back(T);
  };

  drawEco();
}

double calcAverage(std::vector<double> const& v) {
  if (v.empty()) {
    return 0;
  }
  double sum = 0;
  for (int i = 0; i < v.size(); i++) {
    sum += v.at(i);
  }
  return sum / v.size();
}

void loop() {
  const int prevEcoY = ecoY;

  getSensoreData();

  if (prevEcoY != ecoY) {
    tft.fillRect(16, prevEcoY, 28, 4, BLACK);
    drawEco();
  };

  averageHeight.emplace_back(H);
  averageHeight.erase(averageHeight.begin());

  averageTemp.emplace_back(T);
  averageTemp.erase(averageTemp.begin());

  tft.setTextSize(4);

  tft.setCursor(258, 76);
  tft.print(calcAverage(averageTemp), 1);
  tft.print(" C");

  tft.setCursor(258, 216);
  tft.print(calcAverage(averageHeight), 0);
  tft.print(" M");

  delay(100);
}

void getSensoreData() {
  status = pressure.startTemperature();
  if (status != 0) {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0) {
      Serial.print("temperature: ");
      Serial.print(T, 2);
      Serial.println(" deg C, ");

      status = pressure.startPressure(3);
      if (status != 0) {
        delay(status);
        status = pressure.getPressure(P, T);
        if (status != 0) {
          Serial.print("absolute pressure: ");
          Serial.print(P, 2);
          Serial.println("mba, ");

          H = ((pow(1013.25 / P, 1 / 5.257) - 1) * (325.15)) / 0.0065;

          // Todo
          p1 = P;
          // if (p1 > 850) {
          //   p1 = 850;
          // } else if (p1 < 350) {
          //   p1 = 350;
          // };

          // ecoY = map(p1, 350, 850, 270, 46);

          ecoY = map(p1, 300, 1010, 270, 46);

        } else Serial.println("error retrieving pressure measurement\n");
      } else Serial.println("error starting pressure measurement\n");
    } else Serial.println("error retrieving temperature measurement\n");
  } else Serial.println("error starting temperature measurement\n");
}

void drawEco() {
  if (ecoY > 200) {
    tft.fillRect(16, ecoY, 28, 4, GREEN);
  } else if (ecoY > 120) {
    tft.fillRect(16, ecoY, 28, 4, YELLOW);
  } else {
    tft.fillRect(16, ecoY, 28, 4, RED);
  }
  tft.setCursor(14, 288);
  tft.setTextSize(3);
  tft.print(P / 10, 0);
}
