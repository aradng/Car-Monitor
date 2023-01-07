/////////////////////////////////////////////////////////////////////////////////
// BMP280_DEV - I2C Communications, Default Configuration, Normal Conversion
/////////////////////////////////////////////////////////////////////////////////

#include <BMP280_DEV.h>                           // Include the BMP280_DEV.h library
#include <Wire.h>

float temperature, pressure, altitude;            // Create the temperature, pressure and altitude variables
BMP280_DEV bmp280(Wire);                                // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)
BMP280_DEV bmp280_1(Wire1);                                // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)
void setup() 
{
  Serial.begin(115200);                           // Initialise the serial port
  bmp280.begin(0x76);                                 // Default initialisation, place the BMP280 into SLEEP_MODE 
  bmp280_1.begin(0x76);                                 // Default initialisation, place the BMP280 into SLEEP_MODE 
  Serial.println("Starting ...");
  //bmp280.setPresOversampling(OVERSAMPLING_X4);    // Set the pressure oversampling to X4
  //bmp280.setTempOversampling(OVERSAMPLING_X1);    // Set the temperature oversampling to X1
  //bmp280.setIIRFilter(IIR_FILTER_4);              // Set the IIR filter to setting 4
  bmp280.setTimeStandby(TIME_STANDBY_250MS);     // Set the standby time to 2 seconds
  bmp280.startNormalConversion();                 // Start BMP280 continuous conversion in NORMAL_MODE  
  bmp280_1.setTimeStandby(TIME_STANDBY_250MS);     // Set the standby time to 2 seconds
  bmp280_1.startNormalConversion();                 // Start BMP280 continuous conversion in NORMAL_MODE  
}

void loop() 
{
  if (bmp280.getMeasurements(temperature, pressure, altitude))    // Check if the measurement is complete
  {
    Serial.print("bmp0: ");
    Serial.print(temperature);                    // Display the results    
    Serial.print(F("*C   "));
    Serial.print(pressure);    
    Serial.print(F("hPa   "));
    Serial.print(altitude);
    Serial.println(F("m"));  
  }
  if (bmp280_1.getMeasurements(temperature, pressure, altitude))    // Check if the measurement is complete
  {
    Serial.print("bmp1: ");
    Serial.print(temperature);                    // Display the results    
    Serial.print(F("*C   "));
    Serial.print(pressure);    
    Serial.print(F("hPa   "));
    Serial.print(altitude);
    Serial.println(F("m"));  
  }
}
