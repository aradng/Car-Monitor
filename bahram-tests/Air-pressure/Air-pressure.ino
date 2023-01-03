
#include <SFE_BMP180.h>
#include <Wire.h>

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
  Serial.println("REBOOT");
  uint16_t ID = tft.readID();


  if (ID == 0xD3D3)
    ID = 0x9481;  // force ID if write-only display
  tft.begin(ID);
  initializeSetup();

  // Initialize the sensor (it is important to get calibration values stored on the device).

  if (pressure.begin())
    Serial.println("BMP180 init success");
  else {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.println("BMP180 init fail\n\n");
    while (1)
      ;  // Pause forever.
  }
}

void initializeSetup() {
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  tft.setTextColor(RED, BLACK);
  tft.setTextSize(6);
}
double p1;
double ecoY;
void loop() {

  tft.setCursor(30, 120);

  char status;
  double T, P, p0, a, H;


  status = pressure.startTemperature();
  if (status != 0) {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0) {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T, 2);
      Serial.println(" deg C, ");
      tft.print(T, 2);
      tft.print(" deg C, ");

      status = pressure.startPressure(3);
      if (status != 0) {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P, T);
        if (status != 0) {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P / 10, 2);
          Serial.println("kPa, ");
          // tft.print(P / 10, 2);
          // tft.print(" KPa, ");

          H = ((pow(1013.25 / P, 1 / 5.257) - 1) * (325.15)) / 0.0065;
          tft.print(H, 0);
          tft.print(" M, ");
          
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

          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb

          // p0 = pressure.sealevel(P, ALTITUDE);  // we're at 1655 meters (Boulder, CO)
          // Serial.print("relative (sea-level) pressure: ");
          // Serial.print(p0,2);
          // Serial.print(" mb, ");
          // Serial.print(p0*0.0295333727,2);
          // Serial.println(" inHg");

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          // a = pressure.altitude(P, 1013.25);
          // Serial.print("computed altitude: ");
          // Serial.print(a, 0);
          // Serial.print(" meters, ");
          // Serial.print(a * 3.28084, 0);
          // Serial.println(" feet");


        } else Serial.println("error retrieving pressure measurement\n");
      } else Serial.println("error starting pressure measurement\n");
    } else Serial.println("error retrieving temperature measurement\n");
  } else Serial.println("error starting temperature measurement\n");

  delay(1000);  // Pause for 5 seconds.
}
