
#include <BMP280_DEV.h>
#include <thermistor.h>
#include <Wire.h>
#include "car-lcd.c"

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
thermistor thermOil(A8, 0);

BMP280_DEV altimeter(Wire1);
BMP280_DEV manifold(Wire);

int ecoY = 270;
int afrY = 270;
float T, P, A, To;
const int n = 5 ;
int i = 0;
double temperature[n];
double altitude[n];
double oilTemp[n];

double avrTemperature;
double prevAvrTemperature;
double avrAltitude;
double prevAvrAltitude;
double avrOilTemp;
double prevOilTemp;

float p_manifold = 0;
float batteryVoltage = 0;
float prevBatteryVoltage = 0;
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
  delay(200);

  Serial.begin(9600);
  uint16_t ID = tft.readID();

  if (ID == 0xD3D3)
    ID = 0x9481;
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
  altimeter.setTimeStandby(TIME_STANDBY_05MS);
  altimeter.startNormalConversion();
  manifold.setTimeStandby(TIME_STANDBY_05MS);
  manifold.startNormalConversion();
  for (int j = 0; j < 4; j++) {
    getSensoreData();
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

  // avrTemperature = calcAverage(temperature);
  avrAltitude = calcAverage(altitude);
  // avrOilTemp = calcAverage(oilTemp);
  drawEco();
  drawAfr();
}

void loop() {
  prevAvrTemperature = avrTemperature;
  prevAvrAltitude = avrAltitude;
  prevOilTemp = To;
  prevBatteryVoltage = batteryVoltage;
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
  if (batteryVoltage < 10 && prevBatteryVoltage > 10) {
    tft.fillRect(78, 76, 100, 36, BLACK);
  }
  setBatteryColor(batteryVoltage);
  tft.setCursor(78, 76);
  tft.print(batteryVoltage, 1);

  // oil temperature
  avrOilTemp = calcAverage(oilTemp);
  if ((To <= 100 && prevOilTemp >= 100)) {
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

  batteryVoltage = analogRead(A9) * 4.3 * 3.3 / 1023;

  v_afr = analogRead(A11) * 3.3 / 1023;
  if (v_afr > 0.8) {
    v_afr = 0.8;
  } else if (v_afr < 0.1) {
    v_afr = 0.1;
  };
  afrY = map(v_afr * 10, 1, 8, 270, 46);

  To = (thermOil.analog2temp() + 273) * 0.84 - 273;
  oilTemp[i % n] = To;

  i++;
}

double calcAverage(double data[]) {
  double sum = 0;
  for (int j = 0; j < n; i++)
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
  if (voltage < 12) {
    tft.setTextColor(YELLOW, BLACK);
  } else if (voltage < 13) {
    tft.setTextColor(BLUE, BLACK);
  } else if (voltage > 14) {
    tft.setTextColor(RED, BLACK);
  } else {
    tft.setTextColor(GREEN, BLACK);
  }
}

void setOilTempColor(double temp) {
  if (temp < 80) {
    tft.setTextColor(BLUE, BLACK);
  } else if (temp > 110) {
    tft.setTextColor(RED, BLACK);
  } else {
    tft.setTextColor(GREEN, BLACK);
  }
}
