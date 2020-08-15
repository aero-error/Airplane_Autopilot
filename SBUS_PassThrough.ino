/*
 * reads SBUS radio input and directly outputs it as a PWM servo output (1000-2000us)
 * if FailSafe is detected the throttle will be set to 0% and the control surfaces 
 * will return to zero deflection (50%)
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

//defining variables
uint16_t channels[16];
bool failSafe;
bool lostFrame;
int val_ale, val_ele, val_thr, val_rud, val_flaps, val_gear, val_ch7, val_ch8, val_flightmode;
int input, I_min, I_max, F_min, F_max, output;
int LED_PIN = 13;

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
SBUS RX(Serial1);
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
  //Serial.begin(115200); //used for debugging purposes
  RX.begin();
  Servo_ALE.attach(9);
  Servo_ELE.attach(8);
  ESC_THR.attach(7);
  Servo_RUD.attach(6);
  Servo_Flaps.attach(5);
  Servo_Gear.attach(4);
  Servo_CH7.attach(3);
  Servo_CH8.attach(2);
  //escCalibration(); // This can be commented out for testing
  digitalWrite(LED_PIN, LOW); // LED low when the calibration is done
}
//*******Main Loop*******
void loop() {
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
  ESC_THR.write(179);
  delay(6000);
  ESC_THR.write(0);
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
