#include "DigiEncoder.h"
#include "driver.h"


L293D driver(8, 9, 10, 11, 3, 12);
DigiEncoder DigiEncoder(13,driver);

void EncoderISR(){
  DigiEncoder.Increase();
}


void setup(){
  DigiEncoder.EncBegin();
  driver.setspeed(130);
  attachInterrupt(digitalPinToInterrupt(13), EncoderISR, RISING);
}

void loop(){
  driver.forward();
  DigiEncoder.StopAt(30);
}