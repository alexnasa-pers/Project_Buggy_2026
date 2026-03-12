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

void DigiEncoder::StopAt(float traveldist) {
  GoalCount = ((traveldist / CIRCUMFERENCE) * 8); // 8 pulses per rev
  if (Count >= GoalCount) {
    driver.setspeed(0);
  }
}

float DigiEncoder::DistancedTravelled(int Count) {
  return ((tofloat(Count)/8)*20.4);
}