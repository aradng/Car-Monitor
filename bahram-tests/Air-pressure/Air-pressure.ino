
#include <SFE_BMP180.h>
#include <Wire.h>
#include <vector>

#include "car-lcd.h"



#include <Adafruit_GFX.h>   // Core graphics library
#include <MCUFRIEND_kbv.h>  // Hardware-specific library
MCUFRIEND_kbv tft;


// You will need to create an SFE_BMP180 object, here called "pressure":

SFE_BMP180 pressure;

#define ALTITUDE 1655.0  // Altitude of SparkFun's HQ in Boulder, CO. in meters


#define BLACK 0x0000
#define RED 0xF800
#define GREEN 0x07E0
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

double p1;
double ecoY;
char status;
double T, P, p0, a;
double H = 0;
std::vector<double> averageHeight;
std::vector<double> averageTemp;


void initializeSetup() {
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  // tft.drawBitmap(0,0,epd_bitmap_car_lcd,480,320,RED);
  // tft.setAddrWindow(0, 0, 480, 320);
  // tft.pushColors(epd_bitmap_car_lcd,153600,1);
  tft.setTextColor(RED, BLACK);
  tft.setTextSize(6);

  for (int i = 0; i < 5; i++) {
    getSensoreData();
    averageHeight.emplace_back(H);
    averageTemp.emplace_back(T);
  };
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
  getSensoreData();

  averageHeight.emplace_back(H);
  averageHeight.erase(averageHeight.begin());

  averageTemp.emplace_back(T);
  averageTemp.erase(averageTemp.begin());

  tft.setCursor(100, 100);
  tft.print(calcAverage(averageTemp), 1);
  tft.print(" C,");

  tft.setCursor(100, 180);
  tft.print(calcAverage(averageHeight), 0);
  tft.print(" M, ");

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
          // p1 = P;
          // if (p1 > 850) {
          //   p1 = 850;
          // } else if (p1 < 350) {
          //   p1 = 350;
          // };

          // tft.drawFastHLine(0, ecoY, 200,BLACK);
          // ecoY = map(p1,350,850,300,20);
          // tft.print(ecoY, 2);
          // tft.print(" Y, ");
          // tft.drawFastHLine(0, ecoY, 200,WHITE);

        } else Serial.println("error retrieving pressure measurement\n");
      } else Serial.println("error starting pressure measurement\n");
    } else Serial.println("error retrieving temperature measurement\n");
  } else Serial.println("error starting temperature measurement\n");
}
