//reads SBUS radio input and directly outputs it as a PWM servo output (1000-2000us)
//if FailSafe is detected the throttle will be set to 0% and the control surfaces 
//will return to zero deflection (50%)
//Designed to run on Arduino Mega

//including libraries
#include "SBUS.h"
#include "Servo.h"

//defining variables
uint16_t channels[16];
bool failSafe;
bool lostFrame;

int val_ale, val_ele, val_thr, val_rud;
int input, I_min, I_max, F_min, F_max, output;
int LED_PIN = 13;

//Creating objects
SBUS XM(Serial1);
Servo Servo_ALE;
Servo Servo_ELE;
Servo ESC_THR;
Servo Servo_RUD;

//*******Starting Procedure*******
void setup() {
  pinMode(LED_PIN, OUTPUT);       
  digitalWrite(LED_PIN, HIGH);    // LED high when startup procedure starts
  Serial.begin(115200); // used for debugging purposes
  XM.begin();
  //Note: Adding an esc calibartion procedure here would not be a bad idea
  Servo_ALE.attach(7);
  Servo_ELE.attach(6);
  ESC_THR.attach(5);
  Servo_RUD.attach(4);
  digitalWrite(LED_PIN, LOW);     // LED low when the calibration is done
}
//*******Main Loop*******
void loop() {
  RX_READ();
  Servo_Write();
  while(failSafe == true) {
   RX_READ();
   FAILSAFE();
  }
  //DEBUG();
}

//defined functions
void RX_READ(){
  if(XM.read(&channels[0], &failSafe, &lostFrame)){
  }
}
void Servo_Write() {
  val_ale = map(channels[0], 172, 1810, 0, 179); //note: this should later be moved to another function
  val_ele = map(channels[1], 172, 1810, 0, 179); //when autonomous features are being implemented
  val_thr = map(channels[2], 172, 1810, 0, 179);
  val_rud = map(channels[3], 172, 1810, 0, 179);
  Servo_ALE.write(val_ale);
  Servo_ELE.write(val_ele);
  ESC_THR.write(val_thr);
  Servo_RUD.write(val_rud);
}
void FAILSAFE() {
  Servo_ALE.write(90); //set offsets
  Servo_ELE.write(90);
  ESC_THR.write(0); //null throttle
  Servo_RUD.write(90);
  Serial.println("WARNING: FAILSAFE");
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
