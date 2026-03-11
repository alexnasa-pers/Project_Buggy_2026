#include <Arduino.h>
#include "driver.h"


// Motor driver

L293D driver(5, 11, 6, 9, 3, 10);


// Silver Challenge encoder

const int ENC_DATA_PIN  = A0;   
const int ENC_CLOCK_PIN = A4;   
const int ENC_LATCH_PIN = A3;  
const int ENC_RESET_PIN = A1;  


const float WHEEL_DIAMETER_CM = 6.5;
const int PULSES_PER_REV = 8;
const float WHEEL_CIRCUMFERENCE_CM = 3.1416 * WHEEL_DIAMETER_CM;
const float DISTANCE_PER_PULSE_CM = WHEEL_CIRCUMFERENCE_CM / PULSES_PER_REV;

float stopOffsetCm = 0.0;

// Encoder functions

void encoderBegin() {
  pinMode(ENC_DATA_PIN, INPUT);
  pinMode(ENC_CLOCK_PIN, OUTPUT);
  pinMode(ENC_LATCH_PIN, OUTPUT);
  pinMode(ENC_RESET_PIN, OUTPUT);

  digitalWrite(ENC_CLOCK_PIN, LOW);
  digitalWrite(ENC_LATCH_PIN, LOW);
  digitalWrite(ENC_RESET_PIN, LOW);
}

void resetCounter() {
  digitalWrite(ENC_RESET_PIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(ENC_RESET_PIN, LOW);
}

byte readCounter() {
  digitalWrite(ENC_LATCH_PIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(ENC_LATCH_PIN, LOW);
  delayMicroseconds(5);

  return shiftIn(ENC_DATA_PIN, ENC_CLOCK_PIN, MSBFIRST);
}

float countToDistanceCm(byte count) {
  return count * DISTANCE_PER_PULSE_CM;
}

int distanceToCount(float targetDistanceCm) {
  float correctedDistance = targetDistanceCm - stopOffsetCm;
  if (correctedDistance < 0.0) {
    correctedDistance = 0.0;
  }
  return (int)(correctedDistance / DISTANCE_PER_PULSE_CM + 0.5);
}

void stopBuggy() {
  driver.brake();
  delay(80);
  driver.coast();
}

void moveForwardDistance(float targetDistanceCm, uint8_t speedValue) {
  int targetCount = distanceToCount(targetDistanceCm);

  resetCounter();
  driver.forward(speedValue);

  while (true) {
    byte count = readCounter();
    float distanceCm = countToDistanceCm(count);

    Serial.print("Count: ");
    Serial.print(count);
    Serial.print("  Distance(cm): ");
    Serial.println(distanceCm);

    if (count >= targetCount) {
      stopBuggy();
      Serial.println("Target reached");
      break;
    }
  }
}

// Test control

bool ranTest = false;

void setup() {
  Serial.begin(9600);

  driver.begin();
  encoderBegin();
  resetCounter();

  Serial.println("Silver Challenge test starting");
  Serial.print("Distance per pulse (cm): ");
  Serial.println(DISTANCE_PER_PULSE_CM);

  delay(2000);
}

void loop() {
  if (!ranTest) {
    ranTest = true;

    Serial.println("Moving forward 30 cm...");
    moveForwardDistance(30.0, 180);

    Serial.println("Test complete");
  }
}
