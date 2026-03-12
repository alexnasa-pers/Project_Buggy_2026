#include "driver.h"
#include <Arduino.h>

L293D::L293D(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t EN12, uint8_t EN34)
  : A1_(a1), A2_(a2), A3_(a3), A4_(a4), EN12_(EN12), EN34_(EN34),
    leftSpeed_(0), rightSpeed_(0) {
}

void L293D::begin() {
  pinMode(A1_, OUTPUT);
  pinMode(A2_, OUTPUT);
  pinMode(A3_, OUTPUT);
  pinMode(A4_, OUTPUT);
  pinMode(EN12_, OUTPUT);
  pinMode(EN34_, OUTPUT);

  leftCoast();
  rightCoast();
}

/*********************DUAL CONTROLS*********************/

void L293D::norm() {
  leftSpeed(255);
  rightSpeed(255);
  leftForward();
  rightForward();
}

void L293D::right_norm() {
  rightSpeed(255);
  rightForward();
}

void L293D::left_norm() {
  leftSpeed(255);
  leftForward();
}

void L293D::norm_offset(float s) {
  uint8_t pwm = (uint8_t)(255 * abs(s));
  leftSpeed(pwm);
  rightSpeed(pwm);

  if (s > 0) {
    leftForward();
    rightForward();
  } else if (s < 0) {
    leftBackward();
    rightBackward();
  } else {
    leftCoast();
    rightCoast();
  }
}

void L293D::setspeed(float s) {
  norm_offset(s);
}

void L293D::setLspeed(float s) {
  uint8_t pwm = (uint8_t)(255 * abs(s));
  leftSpeed(pwm);

  if (s > 0) {
    leftForward();
  } else if (s < 0) {
    leftBackward();
  } else {
    leftCoast();
  }
}

void L293D::setRspeed(float s) {
  uint8_t pwm = (uint8_t)(255 * abs(s));
  rightSpeed(pwm);

  if (s > 0) {
    rightForward();
  } else if (s < 0) {
    rightBackward();
  } else {
    rightCoast();
  }
}

void L293D::forward() {
  leftForward();
  rightForward();
}

void L293D::forward(uint8_t s) {
  speed(s);
  leftForward();
  rightForward();
}

void L293D::backward() {
  leftBackward();
  rightBackward();
}

void L293D::backward(uint8_t s) {
  speed(s);
  leftBackward();
  rightBackward();
}

void L293D::brake() {
  leftBrake();
  rightBrake();
}

void L293D::coast() {
  leftCoast();
  rightCoast();
}

void L293D::speed(uint8_t s) {
  leftSpeed(s);
  rightSpeed(s);
}

/*********************ONE-SIDED CONTROLS*********************/

void L293D::leftSpeed(uint8_t s) {
  analogWrite(EN12_, s);
  leftSpeed_ = s;
}

void L293D::rightSpeed(uint8_t s) {
  analogWrite(EN34_, s);
  rightSpeed_ = s;
}

void L293D::leftCoast() {
  leftSpeed(0);
  writeLeft(LOW, LOW);
}

void L293D::rightCoast() {
  rightSpeed(0);
  writeRight(LOW, LOW);
}

void L293D::leftBrake() {
  leftSpeed(255);
  writeLeft(HIGH, HIGH);
  delay(50);
  leftCoast();
}

void L293D::rightBrake() {
  rightSpeed(255);
  writeRight(HIGH, HIGH);
  delay(50);
  rightCoast();
}

void L293D::leftForward() {
  writeLeft(HIGH, LOW);
}

void L293D::rightForward() {
  writeRight(HIGH, LOW);
}

void L293D::leftBackward() {
  writeLeft(LOW, HIGH);
}

void L293D::rightBackward() {
  writeRight(LOW, HIGH);
}

/*********************INTERNAL*********************/

void L293D::writeLeft(bool a1, bool a2) const {
  digitalWrite(A1_, a1);
  digitalWrite(A2_, a2);
}

void L293D::writeRight(bool a3, bool a4) const {
  digitalWrite(A3_, a3);
  digitalWrite(A4_, a4);
}
