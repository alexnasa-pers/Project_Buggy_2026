#pragma once
#include <Arduino.h>
#include "driver.h"

class DigiEncoder{
private:
  int Pin;
  
  int GoalCount;
  const float CIRCUMFERENCE = 20.4;
  float traveldist;
  L293D &driver; 
public:
  int Count;
  DigiEncoder(int Pin, L293D &drv);
  void EncBegin();
  void Increase();
  void StopAt(float traveldist);
};



