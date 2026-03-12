#include "driver.h"
#include <Arduino.h>

  L293D::L293D(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t EN12, uint8_t EN34)
  : A1_(a1), A2_(a2), A3_(a3), A4_(a4), EN12_(EN12), EN34_(EN34) {
    //empty
  }

  void L293D::begin(){
    pinMode(A1_, OUTPUT);
    pinMode(A2_, OUTPUT);
    pinMode(A3_, OUTPUT);
    pinMode(A4_, OUTPUT);
    pinMode(EN12_, OUTPUT);
    pinMode(EN34_, OUTPUT);
  }

  void L293D::norm(){
    leftSpeed(255);
    rightSpeed(255);
    rightForward();
    leftForward();
  }
  
  void L293D::right_norm(){
    rightSpeed(245);
    rightForward();
  }

  void L293D::left_norm(){
    leftSpeed(185);
    leftForward();
  }
    void L293D::norm_offset(float s){
    leftSpeed(255*abs(s));
    rightSpeed(255*abs(s));
    if (s>0){
    rightForward();
    leftForward();
    }
    else if (s<0){
    rightBackward();
    leftBackward();
    }
  }

  void L293D::setspeed(float s){
    norm_offset(s);
  }


  void L293D::setLspeed(float s){
    leftSpeed(155*abs(s));
    if (s>0){
    leftForward();
    }
    else if (s<0){
    leftBackward();
    }
  }

  void L293D::setRspeed(float s){
    rightSpeed(225*abs(s));
    if (s>0){
    rightForward();
    }
    else if (s<0){
    rightBackward();
    }
  }  
  void L293D::forward(){
    leftForward();
    rightForward();
  }

  // void L293D::forward(uint8_t s){
  //   speed(s, s);
  //   leftForward();
  //   rightForward();
  // }

  void L293D::forward(uint8_t s){
    speed(s);
    leftForward();
    rightForward();
  }

  void L293D::backward(){
    leftBackward();
    rightBackward();
  }

  // void L293D::backward(uint8_t s){
  //   speed(s);
  //   leftBackward();
  //   rightBackward();
  // }

  void L293D::backward(uint8_t s){
    speed(s);
    leftBackward();
    rightBackward();
  }

  void L293D::brake(){
    leftBrake();
    rightBrake();
  }

  void L293D::coast(){
    leftCoast();
    rightCoast();
  }

  void L293D::speed(uint8_t s){
    leftSpeed(0.8*s);
    rightSpeed(s);
  }

  void L293D::leftSpeed(uint8_t s){
    //Use analogWrite() to give Pulse-width modulation
    analogWrite(EN12_, s);
    leftSpeed_ = s;
  }

  void L293D::rightSpeed(uint8_t s){
    //Use analogWrite() to give Pulse-width modulation
    analogWrite(EN34_, s);
    rightSpeed_ = s;
  }

  void L293D::leftCoast(){
    //1. Set speed to 0: turns off PWM on the enable pin
    leftSpeed(0);
    //2. Set digital pins to actual coasting state
    writeLeft(LOW, LOW);
  }

  void L293D::rightCoast(){
    //1. Set speed to 0: turns off PWM on the enable pin
    rightSpeed(0);
    //2. Set digital pins to actual coasting state
    writeRight(LOW, LOW);
  }

  void L293D::leftBrake(){
    //1. Set PWM to 100% duty cycle to give max braking (this isn't very elegent but probably grand)
    leftSpeed(255);
    //2. Set to brake mode
    writeLeft(HIGH,HIGH);
    leftCoast();
    //3. Wait before setting speed to 0
    //delay(deadtime);
    //leftCoast();
  }

  void L293D::rightBrake(){
    //1. Set PWM to 100% duty cycle to give max braking (this isn't very elegent but probably grand)
    rightSpeed(255);
    //2. Set to brake mode
    writeRight(HIGH, HIGH);
    rightCoast();
    //3. Wait before setting speed to 0
    //delay(deadtime);
    //rightCoast();
  }

  void L293D::leftForward(){
    //1.Disable motor to avoid shoot-through
    //leftCoast();
    //delay(L293D_DEADTIME);
    //2. Set new direction
    writeLeft(HIGH, LOW);//NOTE: this may need changing depending on orientation of motor
    //3. Reset speed
    //leftSpeed(leftSpeed_);
  }

  void L293D::rightForward(){
    //1.Disable motor to avoid shoot-through
    //rightCoast();
    //delay(L293D_DEADTIME);
    //2. Set new direction
    writeRight(HIGH, LOW);//NOTE: this may need changing depending on orientation of motor
    //3. Reset speed
    //rightSpeed(rightSpeed_);
  }

  void L293D::leftBackward(){
    //1.Disable motor to avoid shoot-through
    //leftCoast();
    //delay(L293D_DEADTIME);
    //2. Set new direction
    writeLeft(LOW, HIGH);//NOTE: this may need changing depending on orientation of motor
    //3. Reset speed
    leftSpeed(leftSpeed_);
  }

  void L293D::rightBackward(){
    //1.Disable motor to avoid shoot-through
    //rightCoast();
    //delay(L293D_DEADTIME);
    //2. Set new direction
    writeRight(LOW, HIGH);//NOTE: this may need changing depending on orientation of motor
    //3. Reset speed
    rightSpeed(rightSpeed_);
  }

/************PRIVATE METHODS************/
  void L293D::writeLeft(bool a1, bool a2) const{
    digitalWrite(A1_, a1);
    digitalWrite(A2_, a2);
  }

  void L293D::writeRight(bool a3, bool a4) const{
    digitalWrite(A3_, a3);
    digitalWrite(A4_, a4);
  }
