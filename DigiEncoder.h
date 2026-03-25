#pragma once
#include <Arduino.h>
#include "driver.h"

class DigiEncoder{
private:
  int Pin;
  
  //int GoalCount;
  int LastCount;
  const float CIRCUMFERENCE = 20.4;
  const int rightTurn90Time = 675;
  const int leftTurn90Time  = 690;
  float traveldist;
  L293D &driver; 
public:
  volatile int Count;
  int GoalCount;
  DigiEncoder(int Pin, L293D &drv);
  void EncBegin();
  void Increase();
  float DistanceTravelled(int Count);
  void RightTurn();
  void LeftTurn();
};

