
#include <BMP280_DEV.h>  // Include the BMP280_DEV.h library
#include <Wire.h>
#include <vector>
#include "car-lcd.c"

#include <Adafruit_GFX.h>   // Core graphics library
#include <MCUFRIEND_kbv.h>  // Hardware-specific library
MCUFRIEND_kbv tft;

BMP280_DEV altimeter(Wire);
BMP280_DEV manifold(Wire1);

int ecoY = 270;
float T, P, A;
std::vector<double> temperature;
std::vector<double> altitude;
float p_manifold = 0;
float v_bat = 0;
float t_water = 0;
float afr = 0;
int i_avr = 0;

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
    temperature.emplace_back(T);
  }
  initializeSetup();
}

void initializeSetup() {
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  // tft.drawRGBBitmap(157, 43, vwLogo, 153, 145);
  // tft.drawRGBBitmap(145, 225, name, 189, 15);

  // delay(1000);

  tft.drawRGBBitmap(0, 0, car_lcd, 480, 320);

  tft.setTextColor(WHITE, BLACK);

  drawEco();
}

void loop() {
  const int prevEcoY = ecoY;

  getSensoreData();

  if (prevEcoY != ecoY) {
    tft.fillRect(16, prevEcoY, 28, 4, BLACK);
    drawEco();
  };

  altitude.emplace_back(A);
  altitude.erase(altitude.begin());

  temperature.emplace_back(T);
  temperature.erase(temperature.begin());

  tft.setTextSize(4);
  
  tft.setCursor(78, 76);
  tft.print(v_bat, 1);
  tft.print(" V");

  tft.setCursor(258, 76);
  tft.print(calcAverage(temperature), 1);
  tft.print(" C");
  Serial.print(calcAverage(temperature));
  Serial.println(" avr temp");

  tft.setCursor(258, 226);
  tft.print(calcAverage(altitude), 0);
  tft.print(" M");
  Serial.print(calcAverage(altitude));
  Serial.println(" avr alt");

  delay(100);
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
  // ecoY = map(p1, 300, 1010, 270, 46);
  ecoY = map(p1, 350, 850, 270, 46);

  v_bat = analogRead(A11)*5*3.3/1023;
  // Serial.print("battery voltage (V): ");
  // Serial.println(v_bat);
  // afr = analogRead(A8);
  // t_water = analogRead(A9);
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
