#pragma once
#include <Arduino.h>

#define L293D_BRAKE_TIME 1000 //default for how long to brake for before coasting
#define L293D_DEADTIME 50 //defines how long to wait before switching into forward/reverse in order to avoid shoot through

//only designed for two-motor set up. 
//Assumes A1-A2 is left motor, A3-A4 is right
class L293D {
private:
  //digital control pins
  uint8_t A1_, A2_, A3_, A4_;
  //Enable pins must be PWM capable (e.g. 3, 5, 6, 9, 10, 11)
  uint8_t EN12_, EN34_;
  //tracks current duty cycle value 0-255
  uint8_t leftSpeed_, rightSpeed_;

public:
  //Assumes 1-2 are on the left side, 3-4 are on the right side
  L293D(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t EN12, uint8_t EN34);
  void begin();//sets pinModes, use in setup()

  /*********************DUAL CONTROLS*********************/
  //higher level methods for controlling both motors
  void norm();
  void left_norm();
  void right_norm();
  void norm_offset(float s);
  void setspeed(float s);
  void atGUIspeed(float s);
  void setRspeed(float s);
  void setLspeed(float s);
  void forward();//keeps current speeds
  void forward(uint8_t speed);//brings both motors to one speed
  //void forward(uint8_t left, uint8_t right);//individual control over speeds

  void backward();
  void backward(uint8_t speed);
  void backward(uint8_t left, uint8_t right);

  void brake();//This relies on a HIGH enable pin (to drive the braking) so waits for deadtime before setting coast()
  void coast();
  void speed(uint8_t s);


  /*********************ONE-SIDED CONTROLS*********************/
  //For finer control over single motor

  //Sets the duty cycle for PWM. 
  //Takes 0-255, 255 being 100% duty cycle (max speed)
  void leftSpeed(uint8_t speed);
  void rightSpeed(uint8_t speed);

  //sets left/right PWM to 0 and sets digital pins to LOW
  void leftCoast();
  void rightCoast();

  //sets left/right motors to brake.
  //This relies on a HIGH enable pin (to drive the braking) so waits for deadtime before setting coast()
  void leftBrake();
  void rightBrake();

  //sets left/right motors forward
  void leftForward();
  void rightForward();

  //sets left/right motors to reverse
  void leftBackward();
  void rightBackward();

private:
  /*********************INTERNAL*********************/
  //write a value to both digitial pins on one side
  void writeLeft(bool a1, bool a2) const;
  void writeRight(bool a3, bool a4) const;
};