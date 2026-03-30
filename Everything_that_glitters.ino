#include "driver.h"
#include "TCRT5000.h"
#include "HCSR04.h"
#include "WiFiS3.h"
#include <regex>
#include <vector>
#include <string>
#include "DigiEncoder.h"

unsigned long now = 0;
bool firstpass = true;
int lastSpeed = 0;
bool printed = false;
char aBuffer[20];
int stoaw(double speed){

        double aw = (0.0887*(pow(speed,3)) - 3.8797*(pow(speed,2)) + 59.598*speed - 172.99);
        return constrain(int(aw), 0, 255);
    }

//bron yallej version
//CHUD-LI CODE!!!
//Driver class constructor 
L293D driver(8, 9, 10, 2, 11, 5);
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
float SpeedVal = 180;
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


double Kp = 1;  //dont change
double Ki = 0.001; //goodfornow
double Kd = 0.00001; //goodfornow

// --- PID variables ---
unsigned long currentTime, previousTime;
double elapsedTime;
double error, lastError;
double input, output, setPoint;
long double cumError, rateError;

double computePID(double inp) {
  currentTime  = millis();
  elapsedTime  = max((double)(currentTime - previousTime), 1.0);

  error     = setPoint - inp;              // both in cm/s
  rateError = (error - lastError) / elapsedTime;

  double out = 10.5 * Kp * error + Ki * cumError + Kd * rateError;

  // Anti-windup: only integrate when not fully saturated
  double unclamped = out;
  out = constrain(out, 0, 255);

  if (unclamped == out) {                  // only if not clamped
    cumError += error * elapsedTime;
    cumError = constrain(cumError, -5000, 5000);
  }

  lastError    = error;
  previousTime = currentTime;
    // Serial.print("[PID] SP=");
    // Serial.print(setPoint, 2);
    // Serial.print("  inp=");
    // Serial.print(inp, 2);
    // Serial.print("  err=");
    // Serial.print(error, 2);
    // Serial.print("  dErr=");
    // Serial.print(dtostrf(rateError, 4,1, aBuffer));
    // Serial.print("  Iterm=");
    // Serial.print(dtostrf(cumError, 2,1,aBuffer));
    // Serial.print("  out(raw)=");
    // Serial.print(out, 2);    // before you map/scale if you still do that
    // Serial.print("  dt(ms)=");
    // Serial.println(elapsedTime, 2);

  else {
  return out;   // PWM
  }
}




// Encoder pins
const int ENC_DATA_PIN  = A0;
const int ENC_CLOCK_PIN = A4;
const int ENC_LATCH_PIN = A3;
const int ENC_RESET_PIN = A1;

// Wheel constants
const float WHEEL_DIAMETER_CM = 6.5;
const int PULSES_PER_REV = 8;
const float WHEEL_CIRCUMFERENCE_CM = 21;
const float DISTANCE_PER_PULSE_CM = 2.625;

//Speed and distance
volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile int oldCount = 0;
volatile float GlobalDistance = 0;
volatile float instantSpeed = 0;
volatile bool speedUpdateNeeded = false;
//debugging
volatile float MoveStartDistance = 0;  // add this global

volatile bool isFired = false;


String mode = "";
//Opens a server on port 5200
WiFiServer server(5200);
WiFiClient client;

void EncoderISR(){
    pulseCount++;
    lastPulseTime = micros();
    speedUpdateNeeded = true;
}
void updateSpeed() {
  static unsigned long lastUpdate   = 0;
  static unsigned long lastCount    = 0;   // total pulses at last update

  unsigned long now = millis();

  // Update every 300 ms
  if (now - lastUpdate >= 1000) {
    float timeSeconds = (now - lastUpdate) / 1000.0;

    // Atomically read the current total count
    noInterrupts();
    unsigned long currentCount = pulseCount;   // pulseCount is the ISR-accumulated total
    interrupts();

    // Pulses since last update
    unsigned long pulsesInPeriod = currentCount - lastCount;

    if (pulsesInPeriod > 0 && timeSeconds > 0) {
      instantSpeed = (pulsesInPeriod * DISTANCE_PER_PULSE_CM) / timeSeconds;
    } else {
      instantSpeed = 0;
    }

    lastCount  = currentCount;
    lastUpdate = now;

    // Debug
    Serial.print("[updateSpeed] pulses: ");
    Serial.print(pulsesInPeriod);
    Serial.print(", time: ");
    Serial.print(timeSeconds, 3);
    Serial.print(", speed: ");
    Serial.println(instantSpeed, 2);
  }
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
        driver.setspeed(1);
        //direction = 1 ;
 
}
// last_direction = direction;
}
//speed variable version of main loop
void DrivingLogic(float SpeedVal){
    if (leftsensor.bright() && rightsensor.dark()){
        driver.setLspeed(SpeedVal);
        driver.setRspeed(1.3*SpeedVal);
        //direction = 2 ;
  
    }
    if (leftsensor.dark() && rightsensor.bright()){
        driver.setLspeed(SpeedVal);
        driver.setRspeed(0.7*SpeedVal);
        //direction = 3;

    }
   
    if (leftsensor.bright() && rightsensor.bright()){
        driver.setLspeed(SpeedVal);
        driver.setRspeed(0*SpeedVal);
        // direction = 5;

        
    }
    if (leftsensor.dark() && rightsensor.dark()){
        driver.setspeed(SpeedVal);
        // direction = 1 ;
  

    }
}


void setup() {
    DigiEncoder.EncBegin();
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

    cmd = "ARRAY: 0:10 10:20 20:30 30:10 40:25 50:15 60:10";
    //cmd = "ARRAY: 0:15 10:15 20:15 30:15 40:15 50:15 60:15";
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
        DrivingLogic(180);
        now = millis();
        if (speeds[0] != 0) {
            previousTime = millis();  // initialise PID timer
            unsigned long segmentStart = millis();

            for (int i = 0; i < 7; i++) {
                    // RESET PID when target changes
                cumError = 0;
                lastError = 0;
                previousTime = millis();
                
                setPoint = speeds[i];
                unsigned long duration = (i == 0) ? (unsigned long)times[0]
                                                : (unsigned long)(times[i] - times[i-1]);
                duration *= 1000UL;
                segmentStart = millis();

                Serial.print("Segment "); Serial.print(i);
                Serial.print(" duration: "); Serial.println(duration);

                while ((millis() - segmentStart) < duration && ctrl) {

                    updateSpeed();

                    input = instantSpeed;
                    SpeedVal = computePID(input/255);
                    
                    // Comprehensive debug every 200ms
                    static unsigned long lastDebug = 0;
                    if (millis() - lastDebug > 5000) {
                        Serial.print("Seg ");
                        Serial.print(i);
                        Serial.print(" | Target: ");
                        Serial.print(speeds[i]);
                        Serial.print("cm/s (");
                        Serial.print(setPoint);
                        Serial.print(") | Actual: ");
                        Serial.print(instantSpeed);
                        Serial.print("cm/s (");
                        Serial.print(input);
                        Serial.print(") | Err: ");
                        Serial.print(error);
                        Serial.print(" | Out: ");
                        Serial.print(SpeedVal);
                        Serial.print(" | Cnt: ");
                        Serial.println(pulseCount);
                        lastDebug = millis();
                    }
                    DrivingLogic(SpeedVal);
                }
                lastSpeed = speeds[i];
            }
        }
            

                        }
                    }


        
        
            

       
    

      
    
    
  

    




    