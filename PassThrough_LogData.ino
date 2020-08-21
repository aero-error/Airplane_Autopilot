/*
 * reads SBUS radio input and directly outputs it as a PWM servo output (1000-2000us)
 * if FailSafe is detected the throttle will be set to 0% and the control surfaces 
 * will return to zero deflection (50%)
 * will also log data from all sensors and inputs to SD card
 * 
 * Designed to run on Arduino Mega
 * 
 * Written by Michael Gromski
 * 
 * serial = debug
 * serial1 = XM
 * serial2 = GPS
 * serial3 = Telemetry
*/

//including libraries
#include "SBUS.h"
#include "Servo.h"
#include "SPI.h"
#include "SD.h"
#include "Wire.h"
#include "SFE_BMP180.h"
#include "TinyGPS++.h"
#include "SoftwareSerial.h"

//defining variables
uint16_t channels[16];
bool failSafe;
bool lostFrame;
int val_ale, val_ele, val_thr, val_rud, val_flaps, val_gear, val_ch7, val_ch8, val_flightmode;
int input, I_min, I_max, F_min, F_max, output;
int LED_PIN = 13;
unsigned long currentTime, previousTime, previousTime1 = 0;


String fileName; //variables for saving data
const int chipSelect = 53; //port for CS
int fileCount = 0;

char status; //variables for barometer
double T, P, altAGL;
float P0 = 1013.25;

static const int RXPin = 11, TXPin = 10; //variables for GPS
static const uint32_t GPSBaud = 9600;
float Lat, Lng;
int Sat, Month, Day, Year, Hour, Minute, Sec;
double gsMPH, gsKnots, gsMPS, gpsALT_m, gpsALT_ft, Heading;

//Create servo offsets
int ALE_offset = 2;
int ELE_offset = 2;
int THR_offset = 20; //this works differently from others
int RUD_offset = 11;
int Flaps_offset = 0;
int Gear_offset = 0;
int CH7_offset = 0;
int CH8_offset = 0;
int flightmode_offset = 0; //note on a different scale from the others 

//Creating objects
File root;
File dataFile;
SFE_BMP180 bmp180;
SBUS RX(Serial1);
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
Servo Servo_ALE;
Servo Servo_ELE;
Servo ESC_THR;
Servo Servo_RUD;
Servo Servo_Flaps;
Servo Servo_Gear;
Servo Servo_CH7;
Servo Servo_CH8;

//*******Starting Procedure*******
void setup() {
  pinMode(LED_PIN, OUTPUT);       
  digitalWrite(LED_PIN, HIGH);  //LED high when startup procedure starts
  Serial.begin(115200); //used for debugging purposes
  RX.begin();
  bmp180.begin();
  getBMP180();
  P0 = P;
  ss.begin(GPSBaud);
  Servo_ALE.attach(9);
  Servo_ELE.attach(8);
  ESC_THR.attach(7);
  Servo_RUD.attach(6);
  Servo_Flaps.attach(5);
  Servo_Gear.attach(4);
  Servo_CH7.attach(3);
  Servo_CH8.attach(2);
  findNumOfFiles();
  //escCalibration(); // This can be commented out for testing
  digitalWrite(LED_PIN, LOW); // LED low when the calibration is done
}
//*******Main Loop*******
void loop() {
  currentTime = micros(); //too not overload the loop function some processes will be timed
  if (abs(currentTime - previousTime) > 5e5) { //every half second run these processes
    DataWrite();
    previousTime = currentTime;
  }
  if (abs(currentTime - previousTime1) > 25e4) { //every quarter second read instruments
    getBMP180();
    GPSread();
    previousTime1 = currentTime;
  }
  RX_READ();
  Servo_Write();
  
  while(failSafe == true) { //FailSafe proceedure
   RX_READ();
   FAILSAFE();
  }
  
  //DEBUG(); // Note: This can be commented out for speed
}

//defined functions
void RX_READ(){
  if(RX.read(&channels[0], &failSafe, &lostFrame)){
  }
  val_ale = map(channels[0], 172, 1810, 0, 179) + ALE_offset;
  val_ele = map(channels[1], 172, 1810, 0, 179) + ELE_offset;
  val_thr = map(channels[2], 172, 1810, 0 + THR_offset, 169); // notice offsets to appease the esc
  val_rud = map(channels[3], 172, 1810, 0, 179) + RUD_offset;
  val_flaps = map(channels[4], 172, 1810, 0, 179) + Flaps_offset; //This will not be used in testing plane
  val_gear = map(channels[5], 172, 1810, 0, 179) + Gear_offset; //This will not be used in testing plane 
  val_ch7 = map(channels[6], 172, 1810, 0, 179) + CH7_offset;  //This will not be used in testing plane
  val_ch8 = map(channels[7], 172, 1810, 0, 179) + CH8_offset;  //This will not be used in testing plane
  val_flightmode = channels[8] + flightmode_offset;
}
void Servo_Write() {
  Servo_ALE.write(val_ale);
  Servo_ELE.write(val_ele);
  ESC_THR.write(val_thr);
  Servo_RUD.write(val_rud);
  Servo_Flaps.write(val_flaps);
  Servo_Gear.write(val_gear);
  Servo_CH7.write(val_ch7);
  Servo_CH8.write(val_ch8);
}
void FAILSAFE() {
  Servo_ALE.write(90); //set offsets
  Servo_ELE.write(90);
  ESC_THR.write(0); //null throttle
  Servo_RUD.write(90);
  Servo_Flaps.write(0); //flaps up
  Servo_Gear.write(0);
  Servo_CH7.write(90);
  Servo_CH8.write(90);
  Serial.println("WARNING: FAILSAFE"); //only for debugging
}
void escCalibration() {
  ESC_THR.write(169);
  delay(6000);
  ESC_THR.write(20);
}
void DEBUG() {
  Serial.print(channels[0]);
  Serial.print(channels[1]);
  Serial.print(channels[2]);
  Serial.println(channels[3]);
  Serial.print(val_ale);
  Serial.print(val_ele);
  Serial.print(val_thr);
  Serial.println(val_rud);
}
void findNumOfFiles() {
  if (!SD.begin(chipSelect)){
    Serial.println("Could not Initialize SD Card!");
    preflightWarning();
  }
  root = SD.open("/");
  numOfFiles(root);
  fileName = "Entry-" + String(fileCount-2) + ".txt";
}

void numOfFiles(File dir) {
  while(true) {
    File entry = dir.openNextFile();
    fileCount++; 
    if (!entry) {
      break; //no more files
    }
  }
  dir.close();
}
void DataWrite () { //write all data you want to save here
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) { 
    dataFile.print(currentTime); //current time (Micros)
    dataFile.print("|");
    dataFile.print(channels[0]); //input roll
    dataFile.print("|");
    dataFile.print(channels[1]); //input pitch
    dataFile.print("|");
    dataFile.print(channels[2]); //input thr
    dataFile.print("|");
    dataFile.print(channels[3]); //input yaw
    dataFile.print("|");
    dataFile.print(channels[4]); //input flaps
    dataFile.print("|");
    dataFile.print(channels[5]); //input gear
    dataFile.print("|");
    dataFile.print(channels[6]); //input Ch7
    dataFile.print("|");
    dataFile.print(channels[7]); //input Ch8
    dataFile.print("|");
    dataFile.print(channels[8]); //input flightmode
    dataFile.print("|");
    dataFile.print(P);
    dataFile.print("|");
    dataFile.print(T);
    dataFile.print("|");
    dataFile.print(altAGL);
    dataFile.print("|");
    dataFile.print(Day);
    dataFile.print("|");
    dataFile.print(Month);
    dataFile.print("|");
    dataFile.print(Year);
    dataFile.print("|");
    dataFile.print(Hour);
    dataFile.print("|");
    dataFile.print(Minute);
    dataFile.print("|");
    dataFile.print(Sec);
    dataFile.print("|");
    dataFile.print(Lat,6);
    dataFile.print("|");
    dataFile.print(Lng,6);
    dataFile.print("|");
    dataFile.print(Heading);
    dataFile.print("|");
    dataFile.print(gpsALT_ft);
    dataFile.print("|");
    dataFile.print(gsMPH);
    dataFile.print("|");
    dataFile.print(Sat);
    dataFile.println("|"); //finish with println
    dataFile.close(); //close file
  }
  else {
    Serial.println("error opening file");
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
        altAGL = bmp180.altitude(P, P0);
      }
    }
  }
  if (status == 0) { //if not getting any signal then display error
    Serial.println("Error Reading BMP180");
  }
}
void GPSInfo() {
  if (gps.location.isValid()) { //location
    Lat = gps.location.lat();
    Lng = gps.location.lng();
  }
  if (gps.date.isValid()) { //date
    Month = gps.date.month();
    Day = gps.date.day();
    Year = gps.date.year();
  }
  if (gps.time.isValid()) { //time
    Hour = gps.time.hour();
    Minute = gps.time.minute();
    Sec = gps.time.second();
  }
  if (gps.satellites.isValid()) { //satellites
    Sat = gps.satellites.value();
  }
  if (gps.course.isValid()) { // heading
    Heading = gps.course.deg();
  }
  if (gps.speed.isValid()) { //ground speed
    gsMPH = gps.speed.mph();
    gsKnots = gps.speed.knots();
    gsMPS = gps.speed.mps();
  }
  if (gps.altitude.isValid()) { //altitude
    gpsALT_m = gps.altitude.meters();
    gpsALT_ft = gps.altitude.feet();
  }
}
void GPSread() {
  while (ss.available() > 0) 
    if (gps.encode(ss.read()))
      GPSInfo();
  
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("Error Reading GPS");
  }
}
void preflightWarning() {
  Serial.println("Preflight Warning!");
  Serial.println("Stopping all processes");
  while(true) {
    digitalWrite(LED_PIN, LOW);
    delay(250);
    digitalWrite(LED_PIN, HIGH);
    delay(250);
  }
}
