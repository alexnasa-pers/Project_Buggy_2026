#include <Arduino.h>
#include "driver.h"
#include "WiFiS3.h"

// Motor driver
L293D driver(8, 9, 10, 2, 5, 11);

//Network info
char ssid[] = "Alexiiphone"; // your network SSID
char pass[] = "alexboiq"; // your network password
//Wifi status, saves as int 0-6 0 = not connected 3 = connected 4 = bad credentials
int status = WL_IDLE_STATUS;

// Encoder pins
const int ENC_DATA_PIN  = A0;
const int ENC_CLOCK_PIN = A4;
const int ENC_LATCH_PIN = A3;
const int ENC_RESET_PIN = A1;

// Wheel constants
const float WHEEL_DIAMETER_CM = 6.5;
const int PULSES_PER_REV = 8;
const float WHEEL_CIRCUMFERENCE_CM = 3.1416 * WHEEL_DIAMETER_CM;
const float DISTANCE_PER_PULSE_CM = WHEEL_CIRCUMFERENCE_CM / PULSES_PER_REV;

// Calibration values
float stopOffsetCm = 2.5;
int rightTurn90Time = 675;
int leftTurn90Time  = 690;
float fractional_turn_time_1 = rightTurn90Time/9;
float fractional_turn_time_2 = leftTurn90Time/9;
bool ranTest = false;

//controls  the "hello client" message 
bool said_hello = false;
//command string
String cmd = "";

//Opens a server on port 5200
WiFiServer server(5200);
WiFiClient client;

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
  if (correctedDistance < 0.0) correctedDistance = 0.0;
  return (int)(correctedDistance / DISTANCE_PER_PULSE_CM + 0.5);
}

void stopBuggy() {
  driver.brake();
  delay(100);
  driver.coast();
}

void moveForwardDistance(float targetDistanceCm, uint8_t speedValue) {
  int targetCount = distanceToCount(targetDistanceCm);
  unsigned long startTime = millis();

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

    if (millis() - startTime > 5000) {
      stopBuggy();
      Serial.println("Timeout reached");
      break;
    }

    delay(10);
  }
}

void turnRight(float turn_time) {
  Serial.println("Turning right 90");
  driver.setLspeed(1.0);
  driver.setRspeed(-1.0);
  delay(turn_time );
  stopBuggy();
}

void turnLeft(float turn_time) {
  Serial.println("Turning left 90");
  driver.setLspeed(-1.0);
  driver.setRspeed(1.0);
  delay(turn_time);
  stopBuggy();
}

void runSilverChallenge() {
  moveForwardDistance(30.0, 180);
  delay(500);

  turnRight(rightTurn90Time);
  delay(500);

  moveForwardDistance(20.0, 180);
  delay(500);

  turnLeft(leftTurn90Time);
  delay(500);

  moveForwardDistance(40.0, 180);
  delay(500);

  Serial.println("Silver challenge sequence complete");
}

void setup() {
  Serial.begin(115200);
  //2sec delay for stability
  delay(2000);
  Serial.println("Booted");
  while (status != 3) { // Attempt to connect to AP
      Serial.print("Attempting to connect to Network named:␣");
      Serial.println(ssid);
      Serial.println(status);
      status = WiFi.begin(ssid, pass);
      delay(1500); // give it time to connect
  }
  //gets arduino IP & prints, if ip != 0.0.0.0, assume a good connection 
  IPAddress ip = WiFi.localIP();
  Serial.print("IP␣Address:␣"); 
  Serial.println(ip);
  server.begin();
  driver.begin();
  encoderBegin();
  resetCounter();

  Serial.println("Silver Challenge test starting");
  Serial.print("Distance per pulse (cm): ");
  Serial.println(DISTANCE_PER_PULSE_CM);

  delay(2000);
}

void loop() {
  //if (!ranTest) {
  //  ranTest = true;
  //  runSilverChallenge();
  //}
  if (!client){client = server.available ();}

  if (!said_hello) {
    client.println("Hello Client");
    said_hello = true;
  }
  
  if (client.available() > 0) {      // <-- only read when data exists
    cmd = client.readStringUntil('\n');
    cmd.trim();
    String temp_string = cmd.substring(1);
    float parameter = temp_string.toFloat();
    Serial.print("Got: ");
    Serial.println(cmd);

    switch(cmd.charAt(0)) {
      case 'F':
        //do some stuff
        client.print("Going Forward");
        Serial.print("Going forward to distance ");
        Serial.print(parameter);
        Serial.println("cm");
        moveForwardDistance(parameter, 180);
        break;
      case 'B':
        //tbc 
        client.print("Going Backwards");
        Serial.print("Going Backwards to distance ");
        Serial.print(parameter);
        Serial.println("cm");
        moveForwardDistance(parameter, -180);
        break;
      case 'R':
        //Turn right by x degrees
        parameter = parameter/10;
        client.print("Going Right");
        Serial.print("Turning right at angle ");
        Serial.print(parameter);
        Serial.println(" degrees");
        turnRight(parameter*fractional_turn_time_1);
        break;
      case 'L':
        //turn left by x degrees
        parameter = parameter/10;
        client.println("Turning Left");
        Serial.print("Turning left at angle ");
        Serial.print(parameter);
        Serial.println(" degrees");
        turnLeft(parameter*fractional_turn_time_2);
        break;
    }
        
  }


}
