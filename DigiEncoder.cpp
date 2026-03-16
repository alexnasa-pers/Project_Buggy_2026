#include "DigiEncoder.h"
#include <Arduino.h>
#include "Driver.h"

DigiEncoder::DigiEncoder(int pin, L293D &drv)
  : Pin(pin), Count(0), GoalCount(0), traveldist(0.0f), driver(drv) {}

void DigiEncoder::EncBegin() {
  pinMode(Pin, INPUT_PULLUP);
}

void DigiEncoder::Increase() {
  Count++;
}

void DigiEncoder::StopAt(float traveldist,float s) {
  driver.forward(s);
  GoalCount = (((traveldist)/ CIRCUMFERENCE) * 8); // 8 pulses per rev
  // if ( Count > (LastCount+2)){
  while(Count < GoalCount){
    Serial.println(GoalCount);
    Serial.println(Count);
  }
  // }
  if (Count >= (GoalCount)) {
    driver.stopBuggy();
    Count = 0;
  }
}

void DigiEncoder::RightTurn(float deg){
  driver.setLspeed(180);
  driver.setRspeed(-180);
  delay(rightTurn90Time * (deg/90));
  driver.stopBuggy();
}

void DigiEncoder::LeftTurn(float deg){
  driver.setLspeed(-180);
  driver.setRspeed(180);
  delay(leftTurn90Time * (deg/90));
  driver.stopBuggy();
}

float DigiEncoder::DistanceTravelled(int Count) {
  return ((float(Count)/8)*20.4);
}