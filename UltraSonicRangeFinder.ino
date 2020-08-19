/*
 * This program reads the distance from the ultrasonic sensor
 * This will be used to get a more accurate AGL measurement below 1 meters
 */
const int triggerPin = 3; //17 for mega, 3 for uno
const int echoPin = 2; //18 for mega, 2 for uno
unsigned long currentTime, previousTime, ct, pt = 0;
float distance;
volatile double t0, t1, highLvlTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(echoPin),readPulse, CHANGE);

}

void loop() {
  currentTime = micros();
  if (abs(currentTime - previousTime) > 1e5) {
    digitalWrite(triggerPin, HIGH);
    ct = micros();
    if (abs(ct - pt) > 15){
      digitalWrite(triggerPin,LOW);
      pt = ct;
      previousTime = currentTime;
    }
  }
  
  distance = highLvlTime * 340 / 2;
  Serial.println(distance);

}
void readPulse() {
  if (digitalRead(echoPin) == HIGH) {
    t0 = micros();
  }
  if (digitalRead(echoPin) == LOW) {
    t1 = micros();
    highLvlTime = (abs(t1 - t0)) * 1e-6;
  }
}
