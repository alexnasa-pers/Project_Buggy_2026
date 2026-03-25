#include "DigiEncoder.h"
#include <Arduino.h>
#include "driver.h"

DigiEncoder::DigiEncoder(int pin, L293D &drv)
  : Pin(pin), Count(0), GoalCount(0), traveldist(0.0f), driver(drv) {}

void DigiEncoder::EncBegin() {
  pinMode(Pin, INPUT_PULLUP);
}

void DigiEncoder::Increase() {
  Count++;
}

void DigiEncoder::RightTurn(){
  driver.setLspeed(180);
  driver.setRspeed(-180);
  delay(rightTurn90Time);
  driver.stopBuggy();
}

void DigiEncoder::LeftTurn(){
  driver.setLspeed(-180);
  driver.setRspeed(180);
  delay(leftTurn90Time);
  driver.stopBuggy();
}

float DigiEncoder::DistanceTravelled(int Count) {
  return ((float(Count)/8)*20.4);
}
