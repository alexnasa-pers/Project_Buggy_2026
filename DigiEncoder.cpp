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
  driver.setspeed(s);
  GoalCount = ((traveldist / CIRCUMFERENCE) * 8); // 8 pulses per rev
  if (Count >= GoalCount) {
    driver.setspeed(0);
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