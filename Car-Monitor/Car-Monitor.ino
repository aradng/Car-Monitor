
#include <BMP280_DEV.h>
#include <thermistor.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

#include "eco-icon.c"
#include "v-icon.c"
#include "temp-icon.c"
#include "oil-icon.c"
#include "m-icon.c"
#include "c-icon.c"
#include "battery-icon.c"
#include "altitude-icon.c"
#include "afr-icon.c"

MCUFRIEND_kbv tft;
thermistor thermOil(A8, 0);

BMP280_DEV altimeter;
BMP280_DEV manifold;

int ecoY = 270;
int afrY = 270;
float T, P, A, To;
const int n = 5;
int i = 0;
double temperature[n];
double altitude[n];
double oilTemp[n];
double batteryVoltage[n];

double avrTemperature;
double prevAvrTemperature;
double avrAltitude;
double prevAvrAltitude;
double avrOilTemp;
double prevAvrOilTemp;

float p_manifold = 0;
float avrBatteryVoltage = 0;
float prevAvrBatteryVoltage = 0;
float v_afr = 0;

#define BLACK 0x0000
#define RED 0xE022
#define GREEN 0x5DE9
#define YELLOW 0xFDE0
#define WHITE 0xFFFF
#define GREY 0x8410
#define BLUE 0x041D

void getSensoreData();

void setup() {
  delay(500);

  Serial.begin(9600);

  tft.reset();
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3)
    ID = 0x9481;
  tft.begin(ID);
  
  if (altimeter.begin(0x77))
    Serial.println("Altimeter Operational.");
  else
    Serial.println("Altimeter init Failed!");

  if (manifold.begin(0x76))
    Serial.println("Manifold Pressure Sensor Operational.");
  else
    Serial.println("Manifold init Failed!");

  altimeter.setTimeStandby(TIME_STANDBY_05MS);
  altimeter.startNormalConversion();
  manifold.setTimeStandby(TIME_STANDBY_05MS);
  manifold.startNormalConversion();

  for (int j = 0; j < 5; j++) {
    getSensoreData();
  }

  initializeSetup();
}

void initializeSetup() {
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  drawBackGround();
  tft.setTextColor(WHITE, BLACK);

  avrAltitude = calcAverage(altitude);

  drawEco();
  drawAfr();
}

void loop() {
  prevAvrTemperature = avrTemperature;
  prevAvrAltitude = avrAltitude;
  prevAvrOilTemp = avrOilTemp;
  prevAvrBatteryVoltage = avrBatteryVoltage;
  const int prevEcoY = ecoY;
  const int prevAfrY = afrY;

  getSensoreData();
  tft.setTextColor(WHITE, BLACK);

  // eco
  if (prevEcoY != ecoY) {
    tft.fillRect(16, prevEcoY, 28, 4, BLACK);
    drawEco();
  }

  // afr
  if (prevAfrY != afrY) {
    tft.fillRect(436, prevAfrY, 28, 4, BLACK);
    drawAfr();
  }

  tft.setTextSize(4);
  // air temperature
  avrTemperature = calcAverage(temperature);
  if ((avrTemperature < 10 && prevAvrTemperature > 10) || avrTemperature > 0 && prevAvrTemperature < 0) {
    tft.fillRect(258, 76, 100, 36, BLACK);
  }
  tft.setCursor(258, 76);
  tft.print(avrTemperature, 1);

  // altitude
  avrAltitude = calcAverage(altitude);
  if ((avrAltitude < 1000 && prevAvrAltitude > 1000) || (avrAltitude < 100 && prevAvrAltitude > 100)) {
    tft.fillRect(258, 226, 100, 36, BLACK);
  }
  setAltitudeColor();
  tft.setCursor(258, 226);
  tft.print(avrAltitude, 0);

  // battery voltage
  avrBatteryVoltage = calcAverage(batteryVoltage);
  if (avrBatteryVoltage < 10 && prevAvrBatteryVoltage > 10) {
    tft.fillRect(78, 76, 100, 36, BLACK);
  }
  setBatteryColor(avrBatteryVoltage);
  tft.setCursor(78, 76);
  tft.print(avrBatteryVoltage, 1);

  // oil temperature
  avrOilTemp = calcAverage(oilTemp);
  if ((To <= 100 && prevAvrOilTemp >= 100)) {
    tft.fillRect(78, 226, 100, 36, BLACK);
  }
  setOilTempColor(avrOilTemp);
  tft.setCursor(78, 226);
  tft.print(avrOilTemp, 0);

  // delay
  delay(300);
}

void getSensoreData() {
  altimeter.getCurrentMeasurements(T, P, A);
  temperature[i % n] = T;
  altitude[i % n] = A;

  manifold.getCurrentPressure(p_manifold);
  float p1 = p_manifold;
  if (p1 > 850) {
    p1 = 850;
  } else if (p1 < 350) {
    p1 = 350;
  };
  ecoY = map(p1, 350, 850, 270, 46);

  batteryVoltage[i % n] = analogRead(A9) * (5.0 / 1023) * 4.3 * 1.04;

  v_afr = analogRead(A11) * (5.0 / 1023);
  if (v_afr > 0.9) {
    v_afr = 0.9;
  } else if (v_afr < 0.1) {
    v_afr = 0.1;
  };
  afrY = map(v_afr * 100, 10, 90, 270, 46);

  To = thermOil.analog2temp();
  oilTemp[i % n] = To;

  i++;
}

double calcAverage(double data[]) {
  double sum = 0;
  for (int j = 0; j < n; j++)
    sum += data[j];
  return sum / n;
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

void setAltitudeColor() {
  if (prevAvrAltitude - avrAltitude < -0.2) {
    tft.setTextColor(GREEN, BLACK);
  } else if (prevAvrAltitude - avrAltitude > 0.2) {
    tft.setTextColor(BLUE, BLACK);
  }
}

void setBatteryColor(float voltage) {
  if (voltage < 11) {
    tft.setTextColor(YELLOW, BLACK);
  } else if (voltage < 12) {
    tft.setTextColor(BLUE, BLACK);
  } else if (voltage > 14.5) {
    tft.setTextColor(RED, BLACK);
  } else {
    tft.setTextColor(GREEN, BLACK);
  }
}

void setOilTempColor(double temp) {
  if (temp < 80) {
    tft.setTextColor(BLUE, BLACK);
  } else if (temp > 125) {
    tft.setTextColor(RED, BLACK);
  } else {
    tft.setTextColor(GREEN, BLACK);
  }
}

void drawBackGround() {
  drawMainBox(70);
  drawMainBox(250);
  drawSideBox(1);
  drawSideBox(2);
  tft.drawRGBBitmap(13, 10, eco_icon, 35, 17);
  tft.drawRGBBitmap(432, 10, afr_icon, 35, 17);
  tft.drawRGBBitmap(133, 21, battery_icon, 40, 31);
  tft.drawRGBBitmap(318, 16, temp_icon, 23, 38);
  tft.drawRGBBitmap(127, 169, oil_icon, 48, 32);
  tft.drawRGBBitmap(308, 167, altitude_icon, 45, 31);
  tft.drawRGBBitmap(203, 95, v_icon, 15, 19);
  tft.drawRGBBitmap(378, 95, c_icon, 23, 17);
  tft.drawRGBBitmap(198, 258, c_icon, 23, 17);
  tft.drawRGBBitmap(381, 258, m_icon, 17, 17);
}

void drawMainBox(int x) {
  int y = 10;
  int w = 160;
  int h = 300;
  tft.fillRoundRect(x - 2, y - 2, w + 4, h + 4, 4, WHITE);
  tft.fillRoundRect(x + 2, y + 2, w - 4, h - 4, 2, BLACK);
  tft.fillRoundRect(x + 20 - 2, y + 150 - 2, 120 + 4, 4, 1, WHITE);
}

void drawSideBox(int j) {
  int x = 10;
  if (j == 2) {
    x = 430;
  }
  int y = 40;
  int w = 40;
  int h = 80;
  tft.fillRoundRect(x - 2, y - 2, w + 4, h + 8, 4, RED);
  if (j == 1) {
    tft.fillRoundRect(x - 2, y + 240 - 84, w + 4, h + 4, 4, GREEN);
    tft.fillRect(x - 2, y + 80, w + 4, h, YELLOW);
  } else {
    tft.fillRoundRect(x - 2, y + 240 - 84, w + 4, h + 4, 4, YELLOW);
    tft.fillRect(x - 2, y + 80, w + 4, h, GREEN);
  }
  tft.fillRoundRect(x + 2, y + 2, w - 4, h * 3 - 6, 2, BLACK);
}