#pragma once
#include <Arduino.h>

class HC_SR04{
  private:
    uint8_t trig_, echo_; 
    bool valid_;
  public:
    HC_SR04(uint8_t trig, uint8_t echo) : trig_(trig), echo_(echo), valid_(true){
      if(trig_ > 19 || echo_ > 19){
        valid_ = false;
      }
      setPinModes();
    }

  float centimeters() const{
    digitalWrite(trig_, LOW);
    delayMicroseconds(2);

    digitalWrite(trig_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_, LOW);

    float duration = pulseIn(echo_, HIGH,3000UL);
    return (duration*0.0343)/2;
  }

  float millimeters() const{
    return centimeters()*10;
  }

  float meters() const{
    return centimeters()/100;
  }
  uint8_t trigPin() const{
    return trig_;
  }

  bool trigPin(uint8_t pin){
    if(pin > 19){
      return false;
    }

    trig_ = pin;
    if(trig_ < 20 || echo_ < 20){
      valid_ = true;
    }
    return true;
  }

  uint8_t echoPin() const{
    return echo_;
  }

  bool echoPin(uint8_t pin){
    if(pin > 19){
      return false;
    }

    echo_ = pin;
    if(trig_ < 20 || echo_ < 20){
      valid_ = true;
    }
    return true;
  }

  void setPinModes() const{
    pinMode(echo_, INPUT);
    pinMode(trig_, OUTPUT);
  }

  operator bool() const{
    return valid_;
  }

};