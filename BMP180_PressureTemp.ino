/*
 * This program reads data from the the BMP180 sensor 
 * Board | SDA | SCL
 * UNO   | A4  | A5
 * MEGA  | 20  | 21
 */
#include <Wire.h>
#include <SFE_BMP180.h>

SFE_BMP180 bmp180;
char status;
double T, P, altAGL;
float Po = 1013.25;
unsigned long currentTime, previousTime = 0; 

void setup() {
  Serial.begin(115200);
  bmp180.begin();
  getBMP180();
  Po = P; //this zeros alt to ground level
}

void loop() {
currentTime = millis();
if (abs(currentTime - previousTime) > 1000){
  getBMP180();
  previousTime = currentTime;
  }
}

void getBMP180() {
    status = bmp180.startTemperature();
    currentTime = millis();

  if (status != 0) {
    status = bmp180.getTemperature(T);
    
    if (status != 0) {
      status = bmp180.startPressure(3);

      if (status != 0) {
        //delay(status);
        status = bmp180.getPressure(P, T);
        altAGL = bmp180.altitude(P, Po);

        if (status != 0) {
          Serial.print("Pressure: ");
          Serial.println(P);

          Serial.print("Temperature: ");
          Serial.println(T);

          Serial.print("Altitude: ");
          Serial.println(altAGL);

          Serial.println("---------");
        }
      }
    }
  }
}
