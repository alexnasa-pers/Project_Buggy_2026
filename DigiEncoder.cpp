#include "DigiEncoder.h"
#include <Arduino.h>
#include "Driver.h"

DigiEncoder::DigiEncoder(int pin, L293D &drv)
  : Pin(pin), Count(0), GoalCount(0), traveldist(0.0f), driver(drv) {}

void DigiEncoder::EncBegin() {
  pinMode(Pin, INPUT);
}

void DigiEncoder::Increase() {
  Count++;
}

void DigiEncoder::StopAt(float traveldist) {
  GoalCount = (CIRCUMFERENCE / traveldist) * 8; // 8 pulses per rev
  if (Count >= GoalCount) {
    driver.setspeed(0);
  }
}