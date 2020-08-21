/*
 * This program will read the analog inputs of the current sensor and the airspeed sensor
 * Then convert the raw values to proper units
 */
const int currentPin = 2, voltagePin = 3, airspeedPin = 4; // need to figure out pin numbers
int rawC, rawV, ravAS;
float current, voltage, airspeed;

void setup() {
Serial.begin(115200);
pinMode(currentPin,INPUT);
pinMode(voltagePin,INPUT);
pinMode(airspeedPin,INPUT);
}

void loop() {
  readPower();
  readAirSpeed();
  Serial.print(current);
  Serial.print(voltage);
  Serial.print(airspeed);
}
void readPower() {
  rawC = analogRead(currentPin);
  current = rawC / 7.4;
  rawV = analogRead(voltagePin);
  voltage = rawV / 12.99;
}
void readAirSpeed() {
   rawAS = analogRead(airspeedPin);
   airspeed = rawAS / 1; //Need to figure out conversion factor
}
