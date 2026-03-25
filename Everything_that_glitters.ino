#include "driver.h"
#include "TCRT5000.h"
#include "HCSR04.h"
#include "WiFiS3.h"
#include <regex>
#include <vector>
#include <string>
#include "DigiEncoder.h"

int now = 0;
bool firstpass = true;
int lastSpeed = 0;
bool printed = false;
//bron yallej version
//CHUD-LI CODE!!!
//Driver class constructor 
L293D driver(8, 9, 6, 10, 2, 5);
//IR sensor constructors
TCRT5000 leftsensor(6);
TCRT5000 rightsensor(7);
//Ultrasonic Sensor constructor
HC_SR04 UltraSensor(13,12);
//DigitalEncoder
DigiEncoder DigiEncoder(3, driver);
//obstacle detection state boolean
bool obstacle = false;
//Network info 
char ssid[] = "Jamie"; // your network SSID
char pass[] = "GimmeDatBuggyUggy"; // your network password
//Wifi status, saves as int 0-6 0 = not connected 3 = connected 4 = bad credentials
int status = WL_IDLE_STATUS;

//Speed control over GUI
int GUISpeed = 0;
float SpeedVal = 0.0;
//checking the ping
bool CheckedPing = false;
//controls the driving loop function 
bool ctrl = true;
//controls  the "hello client" message 
bool said_hello = false;
//Strings received from processing
String cmd = "";

//Opens a server on port 5200

int times[7] = {0, 0, 0, 0, 0, 0 ,0};
int speeds[7] = {0, 0, 0, 0, 0, 0, 0};

// --- PID constants (tune these for your system) ---


double Kp = 2.0;
double Ki = 5.0;
double Kd = 1.0;

// --- PID variables ---
unsigned long currentTime, previousTime;
double elapsedTime;
double error, lastError;
double input, output, setPoint;
double cumError, rateError;


double computePID(double inp) {
  currentTime = millis();
  elapsedTime = (double)(currentTime - previousTime);

  error = setPoint - inp;
  cumError += error * elapsedTime;
  rateError = (error - lastError) / elapsedTime;

  double out = Kp * error + Ki * cumError + Kd * rateError;

  // --- Prevent integral wind-up ---
  out = constrain(out, 0, 255);  

  lastError = error;
  previousTime = currentTime;

  return out;
}



// Encoder pins
const int ENC_DATA_PIN  = A0;
const int ENC_CLOCK_PIN = A4;
const int ENC_LATCH_PIN = A3;
const int ENC_RESET_PIN = A1;

// Wheel constants
const float WHEEL_DIAMETER_CM = 6.5;
const int PULSES_PER_REV = 8;
const float WHEEL_CIRCUMFERENCE_CM = 20.4;
const float DISTANCE_PER_PULSE_CM = 2.55;

//Speed and distance
volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile int oldCount = 0;
volatile float GlobalDistance = 0;
volatile float instantSpeed = 0;

//debugging
volatile float MoveStartDistance = 0;  // add this global

volatile bool isFired = false;


String mode = "";
//Opens a server on port 5200
WiFiServer server(5200);
WiFiClient client;

void EncoderISR(){
  unsigned long now = micros();
  //int currentCount = DigiEncoder.Count;
  byte count = readCounter();
  if (lastPulseTime != 0) {
    pulseInterval = now - lastPulseTime;
    if (pulseInterval > 0) {
      float timeSec = pulseInterval / 1000000.0;
      instantSpeed = DISTANCE_PER_PULSE_CM / timeSec;
    }
  } 


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



byte readCounter() {
  digitalWrite(ENC_LATCH_PIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(ENC_LATCH_PIN, LOW);
  delayMicroseconds(5);

  return shiftIn(ENC_DATA_PIN, ENC_CLOCK_PIN, MSBFIRST);
}

void parseArray(const arduino::String& input)
{
    std::string c_input(input.c_str()); //convert arduino string to c style  string 

    std::regex pair_re(R"((-?\d+\.?\d*):(-?\d+\.?\d*))");
    std::sregex_iterator begin(c_input.begin(), c_input.end(), pair_re);
    std::sregex_iterator end ;
    int i = 0;
    for (std::sregex_iterator it = begin; it != end && i < 7 ; it++, i++) {
        times[i] = (std::stoi((*it)[1].str()));
        speeds[i] = (std::stoi((*it)[2].str()));
    }
}



//obstacle detection send timing
unsigned long lastStatusTime = 0;
const unsigned long statusInterval = 300;
bool sendObst = false;
//int direction = 0; //1=forward , 2=left , 3= right 4= stopped 5 = circling 
//int last_direction = 10 ;
//main driving logic loop
void DrivingLogic(){
    //turns left if left is bright
    if (leftsensor.bright() && rightsensor.dark()){
        driver.setLspeed(0.6);
        driver.setRspeed(1.3);
        //direction = 2 ;

   
       

    }
    //turns right if right is bright
    if (leftsensor.dark() && rightsensor.bright()){
        driver.setLspeed(1.3);
        driver.setRspeed(0.6);
        //direction = 3;
        

        
    }
    //circles if both are bright
    if (leftsensor.bright() && rightsensor.bright()){
        driver.setLspeed(0);
        driver.setRspeed(0.5);
        //direction = 5;

        
    }
    //straight ahead if both are dark
    if (leftsensor.dark() && rightsensor.dark()){
        driver.setspeed(0.8);
        //direction = 1 ;
 
}
// last_direction = direction;
}
//speed variable version of main loop
void DrivingLogic(float SpeedVal){
    if (leftsensor.bright() && rightsensor.dark()){
        driver.setLspeed(0.6*SpeedVal);
        driver.setRspeed(1.3*SpeedVal);
        //direction = 2 ;
  
    }
    if (leftsensor.dark() && rightsensor.bright()){
        driver.setLspeed(1.3*SpeedVal);
        driver.setRspeed(0.4*SpeedVal);
        //direction = 3;

    }
   
    if (leftsensor.bright() && rightsensor.bright()){
        driver.setLspeed(0*SpeedVal);
        driver.setRspeed(0.5*SpeedVal);
        // direction = 5;

        
    }
    if (leftsensor.dark() && rightsensor.dark()){
        driver.setspeed(0.8*SpeedVal);
        // direction = 1 ;
  

    }
}


void setup() {
     // Target position = 0 degrees
    attachInterrupt(digitalPinToInterrupt(3), EncoderISR, CHANGE);
    //open serial port at baud rate 115200
    Serial.begin(115200);
    //2sec delay for stability
    delay(2000);
    //lets us know its on
    Serial.println("Booted");
    //sets the pinmodes and stuff for the motors
    driver.begin();
    //while the wifi is not properly connected 
    // while (status != 3) { // Attempt to connect to AP
    //     Serial.print("Attempting to connect to Network named:␣");
    //     Serial.println(ssid);
    //     Serial.println(status);
    //     status = WiFi.begin(ssid, pass);
    //     delay(1500); // give it time to connect
    // }
    // //gets arduino IP & prints, if ip != 0.0.0.0, assume a good connection 
    // IPAddress ip = WiFi.localIP();
    // Serial.print("IP␣Address:␣"); 
    // Serial.println(ip);
    // server.begin();



}

void loop() {

    cmd = "ARRAY: 0:10 10:20 20:10 30:10 40:30 50:10 60:20";
    parseArray(cmd);
    if (!printed){
    for (int j= 0; j < 7; j++){
        Serial.println(times[j]);
    }
    Serial.println();
    for (int k = 0; k < 7; k++){
        Serial.println(speeds[k]);
        }
    printed = true;
    }
    // if (!client){client = server.available ();}

    // if (client) {   
    //     if (!said_hello) {
    //         client.println("Hello Client");
    //         said_hello = true;
    //     }
    // }
  
    if(speeds[0] == 0){
        Serial.println("hey");
        ctrl = false;
        cmd = client.readStringUntil('\n');
        if (cmd.substring(0,6) == ("ARRAY:")){
            parseArray(cmd);
            ctrl = true;
        }
    }
    ctrl = true;
    if (speeds[0] != 0){
        Serial.println("1");
        DrivingLogic(speeds[0]);
        now = millis();
        for (int i = 0; i < 7; i++){
            Serial.println("2");
            if (lastSpeed != 0 && lastSpeed != speeds[i]){
                firstpass = true;
            }
            if (firstpass){
            DrivingLogic(speeds[i]);
            firstpass = false;
            Serial.println("3");

            }
            while ((now - (times[i]*1000) > 0) && ctrl ){
              
                now = millis();
                setPoint = speeds[i];
                input = instantSpeed;        // Read sensor or encoder input
                SpeedVal = computePID(input);    // Get PID output
                //Serial.println(SpeedVal);
                Serial.println(instantSpeed);
                Serial.println(DigiEncoder.Count);
                DrivingLogic(SpeedVal);
                if ((now - (times[i]*1000) == 30)){
                    Serial.println("5");
                    lastSpeed = speeds[i];
                    }

            

                        }
                    }

                }
            }
        
            

       
    

      
    
    
  

    




    