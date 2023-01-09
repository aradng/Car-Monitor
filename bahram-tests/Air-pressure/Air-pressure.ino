
#include <BMP280_DEV.h>  // Include the BMP280_DEV.h library
#include <Wire.h>
#include "car-lcd.c"

#include <Adafruit_GFX.h>   // Core graphics library
#include <MCUFRIEND_kbv.h>  // Hardware-specific library
MCUFRIEND_kbv tft;

BMP280_DEV altimeter(Wire);
BMP280_DEV manifold(Wire1);

int ecoY = 270;
const int N_temp = 5, N_pres = 5, N_alt = 5;
float temperature[N_temp], pressure[N_pres], altitude[N_alt];
float p_manifold = 0;
float v_bat = 0;
float t_water = 0;
float afr = 0;
int i_temp = 0, i_pres = 0, i_alt = 0;

#define BLACK 0x0000
#define RED 0xE022
#define GREEN 0x5DE9
#define YELLOW 0xFDE0
#define WHITE 0xFFFF
#define GREY 0x8410

float calcAverage(float t[], int n);
void getSensoreData();

void setup() {
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
  altimeter.setTimeStandby(TIME_STANDBY_250MS);  // Set the standby time to 2 seconds
  altimeter.startNormalConversion();             // Start BMP280 continuous conversion in NORMAL_MODE
  manifold.setTimeStandby(TIME_STANDBY_250MS);   // Set the standby time to 2 seconds
  manifold.startNormalConversion();              // Start BMP280 continuous conversion in NORMAL_MODE
  initializeSetup();
}

void initializeSetup() {
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  tft.drawRGBBitmap(157, 43, vwLogo, 153, 145);
  tft.drawRGBBitmap(145, 225, name, 189, 15);

  delay(3000);

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

  tft.setTextSize(4);

  tft.setCursor(258, 76);
  tft.print(calcAverage(temperature, N_temp), 1);
  tft.print(" C");

  tft.setCursor(258, 216);
  tft.print(calcAverage(altitude, N_alt), 0);
  tft.print(" M");

  delay(100);
}

void getSensoreData() {
  float t, p, a;
  if (altimeter.getMeasurements(t, p, a)) {
    temperature[i_temp % N_temp] = t;
    pressure[i_pres % N_pres] = p;
    altitude[i_alt % N_alt] = a;
    i_temp++;
    i_pres++;
    i_alt++;
  }
  manifold.getCurrentPressure(p_manifold);
  ecoY = map(p_manifold, 300, 1010, 270, 46);

  v_bat = analogRead(A11)*5*3.3/1023;
  Serial.print("battery voltage (V): ");
  Serial.println(v_bat);
  afr = analogRead(A8);
  t_water = analogRead(A9);
}

float calcAverage(float t[], int n) {
  double sum = 0;
  for (int i = 0; i < n; i++)
    sum += t[i];
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
