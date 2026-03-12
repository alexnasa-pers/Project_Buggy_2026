#include "DigiEncoder.h"
#include "Driver.h"


L293D driver(10, 11, 8, 9, 2, 12);
DigiEncoder DigiEncoder(3,driver);

void EncoderISR(){
  DigiEncoder.Increase();
}


void setup(){
  Serial.begin(115200);
  DigiEncoder.EncBegin();
  driver.begin();
  driver.setspeed(220);
  attachInterrupt(digitalPinToInterrupt(3), EncoderISR, CHANGE);
}

void loop(){
  driver.forward();
  if(DigiEncoder.Count > 0)
  Serial.println(DigiEncoder.Count);
  DigiEncoder.StopAt(8);
}