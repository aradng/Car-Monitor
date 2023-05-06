
#include <BMP280_DEV.h>  // Include the BMP280_DEV.h library
#include <Wire.h>
#include <vector>
#include "car-lcd.c"

#include <Adafruit_GFX.h>   // Core graphics library
#include <MCUFRIEND_kbv.h>  // Hardware-specific library
MCUFRIEND_kbv tft;

BMP280_DEV altimeter(Wire1);
BMP280_DEV manifold(Wire);

int i = 1;
int ecoY = 270;
int afrY = 270;
float T, P, A;
std::vector<double> temperature;
std::vector<double> altitude;
std::vector<double> oilTemp;

float p_manifold = 0;
float v_bat = 0;
float v_afr = 0;

int Vo;
float R1 = 10000;
float logR2, R2, To;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

#define BLACK 0x0000
#define RED 0xE022
#define GREEN 0x5DE9
#define YELLOW 0xFDE0
#define WHITE 0xFFFF
#define GREY 0x8410

void getSensoreData();

void setup() {
  delay(100);

  Serial.begin(9600);
  uint16_t ID = tft.readID();

  if (ID == 0xD3D3)
    ID = 0x9481;  // force ID if write-only display
  tft.begin(ID);
  if (altimeter.begin(0x76))
    Serial.println("Altimeter Operational.");
  else {
    Serial.println("Altimeter init Failed!");
  }

  if (manifold.begin(0x76))
    Serial.println("Manifold Pressure Sensor Operational.");
  else {
    Serial.println("Manifold init Failed!");
  }
  altimeter.setTimeStandby(TIME_STANDBY_05MS);  // Set the standby time to 2 seconds
  altimeter.startNormalConversion();            // Start BMP280 continuous conversion in NORMAL_MODE
  manifold.setTimeStandby(TIME_STANDBY_05MS);   // Set the standby time to 2 seconds
  manifold.startNormalConversion();             // Start BMP280 continuous conversion in NORMAL_MODE
  for (int j = 0; j < 4; j++) {
    getSensoreData();
    altitude.emplace_back(A);
    oilTemp.emplace_back(To);
    temperature.emplace_back(T);
  }
  initializeSetup();
}

void initializeSetup() {
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  // tft.drawRGBBitmap(157, 43, vwLogo, 153, 145);
  // tft.drawRGBBitmap(145, 225, name, 189, 15);

  // delay(1000);

  tft.drawRGBBitmap(0, 0, car_lcd, 480, 320);

  tft.setTextColor(WHITE, BLACK);

  drawEco();
  drawAfr();
}

void loop() {
  const int prevEcoY = ecoY;
  const int prevAfrY = afrY;
  i++;

  getSensoreData();

  if (prevEcoY != ecoY) {
    tft.fillRect(16, prevEcoY, 28, 4, BLACK);
    drawEco();
  };
  if (prevAfrY != afrY) {
    tft.fillRect(436, prevAfrY, 28, 4, BLACK);
    drawAfr();
  };
  if (i > 30) {
    i = 1;
    tft.fillRect(78, 76, 100, 36, BLACK);
    tft.fillRect(78, 226, 100, 36, BLACK);
    tft.fillRect(258, 76, 100, 36, BLACK);
    tft.fillRect(258, 226, 100, 36, BLACK);
  };

  altitude.emplace_back(A);
  altitude.erase(altitude.begin());

  oilTemp.emplace_back(To);
  oilTemp.erase(oilTemp.begin());

  temperature.emplace_back(T);
  temperature.erase(temperature.begin());

  tft.setTextSize(4);

  tft.setCursor(78, 76);
  tft.print(v_bat, 1);

  tft.setCursor(78, 226);
  tft.print(calcAverage(oilTemp) - 273.15, 0);

  tft.setCursor(258, 76);
  tft.print(calcAverage(temperature), 1);

  tft.setCursor(258, 226);
  tft.print(calcAverage(altitude), 0);

  delay(200);
}

void getSensoreData() {
  altimeter.getCurrentMeasurements(T, P, A);

  manifold.getCurrentPressure(p_manifold);
  float p1 = p_manifold;
  if (p1 > 850) {
    p1 = 850;
  } else if (p1 < 350) {
    p1 = 350;
  };
  ecoY = map(p1, 350, 850, 270, 46);

  v_bat = analogRead(A9) * 4.3 * 3.3 / 1023;

  v_afr = analogRead(A11) * 3.3 / 1023;
  if (v_afr > 0.8) {
    v_afr = 0.8;
  } else if (v_afr < 0.1) {
    v_afr = 0.1;
  };
  afrY = map(v_afr * 10, 1, 8, 270, 46);

  Vo = analogRead(A8);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  To = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2)) * 0.9;
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
  tft.print(p_manifold / 10, 0);
}

void drawAfr() {
  if (afrY > 200) {
    tft.fillRect(436, afrY, 28, 4, YELLOW);
  } else if (afrY > 120) {
    tft.fillRect(436, afrY, 28, 4, GREEN);
  } else {
    tft.fillRect(436, afrY, 28, 4, RED);
  }
  tft.setCursor(434, 288);
  tft.setTextSize(3);
  tft.print(v_afr * 10, 0);
}
