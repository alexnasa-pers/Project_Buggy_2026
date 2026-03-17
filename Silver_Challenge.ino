#include <Arduino.h>
#include "driver.h"
#include "WiFiS3.h"
#include "DigiEncoder.h"
#include "HCSR04.h"

// Motor driver
L293D driver(8, 9, 10, 2, 5, 11);
//Ultrasonic Sensor constructor
HC_SR04 UltraSensor(13,12);
//DigitalEncoder
DigiEncoder DigiEncoder(3, driver);
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

//Speed and distance
volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile int oldCount = 0;
volatile float GlobalDistance = 0;
volatile float instantSpeed = 0;
volatile bool obstacle = false;
//debugging
volatile bool isFired = false;

// Calibration values
float stopOffsetCm = 2.5;
int rightTurn90Time = 455;
int leftTurn90Time  = 445;
float fractional_turn_time_1 = rightTurn90Time/9;
float fractional_turn_time_2 = leftTurn90Time/9;
bool ranTest = false;

//controls  the "hello client" message 
bool said_hello = false;
//command string
String cmd = "";
//Digital or Analogue Control booleans
bool DigiControl = false;
bool AnaControl = true;
//default driver speed
const int DEFAULT_SPEED = 180;
//mode string
String mode = "";
//Opens a server on port 5200
WiFiServer server(5200);
WiFiClient client;

void EncoderISR(){
  unsigned long now = micros();
  
  if (lastPulseTime != 0) {
    pulseInterval = now - lastPulseTime;
    if (pulseInterval > 0) {
      float timeSec = pulseInterval / 1000000.0;
      instantSpeed = DISTANCE_PER_PULSE_CM / timeSec;
    }
  } 

  GlobalDistance += DISTANCE_PER_PULSE_CM;
  DigiEncoder.Increase();
  isFired = true;
  lastPulseTime = now;
}


void encoderBegin() {
  pinMode(ENC_DATA_PIN, INPUT);
  pinMode(ENC_CLOCK_PIN, OUTPUT);
  pinMode(ENC_LATCH_PIN, OUTPUT);
  pinMode(ENC_RESET_PIN, OUTPUT);

  digitalWrite(ENC_CLOCK_PIN, LOW);
  digitalWrite(ENC_LATCH_PIN, LOW);
  digitalWrite(ENC_RESET_PIN, LOW);
}

void StopAt(float traveldist,float s) {
  driver.forward(s);
  DigiEncoder.GoalCount = (((traveldist)/ WHEEL_CIRCUMFERENCE_CM) * 8); // 8 pulses per rev
  // if ( Count > (LastCount+2)){
  unsigned long startTime = millis();

  while(true) {

    delay(20);
    Serial.print("Count: ");
    Serial.print(DigiEncoder.Count);
    Serial.print(" / ");
    Serial.println(DigiEncoder.GoalCount);
    client.println("DIST:" + String(GlobalDistance));
    client.println("SPD:" + String(instantSpeed));
    float distance = UltraSensor.centimeters();
    obstacle = (distance < 10 && distance > 0); 
    if (obstacle) {
      driver.stopBuggy();
      client.println("Obstacle Detected");
    }
    if (DigiEncoder.Count >= (DigiEncoder.GoalCount)) {
      driver.stopBuggy();
      DigiEncoder.Count = 0;
      Serial.println("Count reached");
      break;
    }
    
    if (millis() - startTime > 20000) {
      driver.stopBuggy();
      Serial.println("Timeout reached");
      break;
    
    }
  }
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



void moveForwardDistance(float targetDistanceCm, uint8_t speedValue) {
  int targetCount = distanceToCount(targetDistanceCm);
  unsigned long startTime = millis();
  byte lastCount = 0;
  //For speed
  unsigned long lastTime = millis(); 

  resetCounter();
  driver.forward(speedValue);

  while (true) {
    //Serial.println("im in moveFOrwardDistance WHile loop");
    byte count = readCounter();
    unsigned long now = millis();
    float distanceCm = countToDistanceCm(count);
    float ultra_distance = UltraSensor.centimeters();
    obstacle = (ultra_distance < 10 && ultra_distance > 0); 
    if (obstacle) {
      driver.stopBuggy();
      client.println("Obstacle Detected");
    }
    //float deltaDistance = 0;

    //Speed Calculation
    //seconds since last check
    /*
    float dt = (now - lastTime)/1000.0;
    if (dt > 0) {
      int deltaCount = count - lastCount;
      //stop it from just printing 0
      if (deltaCount > 0) {
      deltaDistance = deltaCount * DISTANCE_PER_PULSE_CM;
      float speed_cm_per_sec = deltaDistance / dt;
      GlobalSpeed = speed_cm_per_sec;
      }
    }
    lastCount = count;
    lastTime = now;
    */
    //GlobalDistance = distanceCm-5;

    Serial.print("Count: ");
    Serial.print(count);
    Serial.print("  Distance(cm): ");
    Serial.println(distanceCm);
    client.println("DIST:" + String(GlobalDistance));
    client.println("SPD:" + String(instantSpeed));

    if ((count >= targetCount) && AnaControl) {
      driver.stopBuggy();
      Serial.println("Target reached");
      break;
    }

    if (millis() - startTime > 30000) {
      driver.stopBuggy();
      Serial.println("Timeout reached");
      break;
    }

    delay(10);
  }
}

void turnRight() {
  Serial.println("Turning right 90");
  driver.setLspeed(1.0);
  driver.setRspeed(-1.0);
  delay(rightTurn90Time);
  driver.stopBuggy();
}

void turnLeft() {
  Serial.println("Turning left 90");
  driver.setLspeed(-1.0);
  driver.setRspeed(1.0);
  delay(leftTurn90Time);
  driver.stopBuggy();
}

void runSilverChallenge() {
  moveForwardDistance(30.0, 180);
  delay(500);

  turnRight();
  delay(500);

  moveForwardDistance(20.0, 180);
  delay(500);

  turnLeft();
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
  //debugging
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(3), EncoderISR, RISING);
  resetCounter();
  
  Serial.println("Silver Challenge test starting");
  Serial.print("Distance per pulse (cm): ");
  Serial.println(DISTANCE_PER_PULSE_CM);
  Serial.print("Circumference: ");
Serial.println(WHEEL_CIRCUMFERENCE_CM);
Serial.print("GoalCount for 20cm: ");
Serial.println((20.0 / WHEEL_CIRCUMFERENCE_CM) * 8);


}

void loop() {
  if(isFired) {
    Serial.println("ISR fired, Count: " + String(DigiEncoder.Count));
    isFired = false;
  }
  //Serial.println(UltraSensor.centimeters());
  //Serial.println(digitalRead(3));
  //delay(100);
  resetCounter();
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
    Serial.println(cmd);
    mode = cmd.substring(5);
    if (mode == "analogue"){
      // Serial.println(AnaControl);
      // Serial.println(DigiControl);

      DigiControl = false;
      AnaControl = true;
      // Serial.println(AnaControl);
      // Serial.println(DigiControl);
      Serial.println("Analogue mode");
    }
    if (mode == "digital"){
      // Serial.println(AnaControl);
      // Serial.println(DigiControl);

      DigiControl = true;
      AnaControl = false;

      // Serial.println(AnaControl);
      // Serial.println(DigiControl);

      Serial.println("Digital mode");
    }

    String temp_string = cmd.substring(1);
    float parameter = temp_string.toFloat();
    
    if (mode == ""){
    Serial.print("Got: ");
    Serial.println(cmd);
    }
    mode = "";
    switch(cmd.charAt(0)) {
      case 'F':
        //do some stuff
        //client.print("Going Forward");
        Serial.print("Going forward to distance ");
        Serial.print(parameter);
        Serial.println("cm");
        if(AnaControl){
          moveForwardDistance(parameter, 180);
        }
        else if(DigiControl){
          StopAt(parameter, DEFAULT_SPEED);
        }
        break;
      case 'B':
        //tbc 
        client.print("Going Backwards");
        Serial.print("Going Backwards to distance ");
        Serial.print(parameter);
        Serial.println("cm");
        if (AnaControl){
          moveForwardDistance(parameter, -180);
        }
        else if (DigiControl){
          StopAt(parameter, (-1*(DEFAULT_SPEED)));
        }
        break;
      case 'R':
        //Turn right by x degrees
        //parameter = parameter/10;
        client.print("Going Right");
        Serial.print("Turning right at angle ");
        Serial.print(parameter);
        Serial.println(" degrees");
        //if (AnaControl){
        turnRight();
        /*}
        else if (DigiControl){
          DigiEncoder.RightTurn();
        }*/
        break;
      case 'L':
        //turn left by x degrees
       // parameter = parameter/10;
        client.println("Turning Left");
        Serial.print("Turning left at angle ");
        Serial.print(parameter);
        Serial.println(" degrees");
        //if (AnaControl){
        turnLeft();
        /*}
        else if (DigiControl){
          DigiEncoder.LeftTurn();
        }*/
        break;
    }
        
  }



}
