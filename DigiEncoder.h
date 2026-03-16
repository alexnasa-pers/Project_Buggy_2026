#pragma once
#include <Arduino.h>
#include "driver.h"

class DigiEncoder{
private:
  int Pin;
  
  int GoalCount;
  int LastCount;
  const float CIRCUMFERENCE = 20.4;
  const int rightTurn90Time = 675;
  const int leftTurn90Time  = 690;
  float traveldist;
  L293D &driver; 
public:
  int Count;
  DigiEncoder(int Pin, L293D &drv);
  void EncBegin();
  void Increase();
  void StopAt(float traveldist,float s);
  float DistanceTravelled(int Count);
  void RightTurn(float deg);
  void LeftTurn(float deg);
};


