#include "driver.h"
#include "TCRT5000.h"
#include "HCSR04.h"
#include "WiFiS3.h"


//bron yallej version
//CHUD-LI CODE!!!
//IR sensor constructors
TCRT5000 leftsensor(12);
TCRT5000 rightsensor(2);
//Ultrasonic Sensor constructor
HC_SR04 UltraSensor(7,8);
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
bool ctrl = false;
//controls  the "hello client" message 
bool said_hello = false;
//Strings received from processing
String cmd = "";
//Driver class constructor 
L293D driver(5, 11, 6, 9, 3, 10);
//Opens a server on port 5200
WiFiServer server(5200);
//main driving logic loop
void DrivingLogic(){
    //turns left if left is bright
    if (leftsensor.bright() && rightsensor.dark()){
       Serial.println("***RIGHT DARK***");
        driver.setLspeed(0.35);
        driver.setRspeed(0.75);
    }
    //turns right if right is bright
    if (leftsensor.dark() && rightsensor.bright()){
       Serial.println("***LEFT DARK***");
        driver.setLspeed(0.75);
        driver.setRspeed(0.35);
    }
    //circles if both are bright
    if (leftsensor.bright() && rightsensor.bright()){
        Serial.println("***ALL BRIGHT***");
        driver.setLspeed(0);
        driver.setRspeed(0.5);
    }
    //straight ahead if both are dark
    if (leftsensor.dark() && rightsensor.dark()){
       Serial.println("***ALL DARK***");
        driver.setspeed(0.8);
}
}
//speed variable version of main loop
void DrivingLogic(float SpeedVal){
    if (leftsensor.bright() && rightsensor.dark()){
       Serial.println("***RIGHT DARK***");
        driver.setLspeed(0.35*SpeedVal);
        driver.setRspeed(0.75*SpeedVal);
    }
    if (leftsensor.dark() && rightsensor.bright()){
       Serial.println("***LEFT DARK***");
        driver.setLspeed(0.75*SpeedVal);
        driver.setRspeed(0.35*SpeedVal);
    }
    if (leftsensor.bright() && rightsensor.bright()){
        Serial.println("***ALL BRIGHT***");
        driver.setLspeed(0*SpeedVal);
        driver.setRspeed(0.5*SpeedVal);
    }
    if (leftsensor.dark() && rightsensor.dark()){
       Serial.println("***ALL DARK***");
        driver.setspeed(0.8*SpeedVal);
}

}
void setup() {
    //open serial port at baud rate 115200
    Serial.begin(115200);
    //2sec delay for stability
    delay(2000);
    //lets us know its on
    Serial.println("Booted");
    //sets the pinmodes and stuff for the motors
    driver.begin();
    //while the wifi is not properly connected 
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


}

void loop() {
    // if true, go , basically 
    if (ctrl){
        DrivingLogic(SpeedVal);
    }
    // sets up wifi client so laptop can connect
    WiFiClient client = server.available ();
    if ( client.connected ()) {
        // sends msg on first connection 
        if (!said_hello){
        client.write("Hello Client\n");
        }
        // reads what the laptop sends over and prints it 
        cmd = client.readString();
        if (cmd != ""){
            Serial.println(cmd);
            //if (cmd == "START\n"){
               // Serial.println("im gonna go");
            }
        }
    
    //stores distance from  ultrasonic 
    float distance = UltraSensor.centimeters();
    // Serial.print("Distance: ");
    // Serial.print(distance);
    // Serial.println(" cm");
    // boolean for obstacle detection 
    obstacle = (distance < 10 && distance > 0);  // Update every loop
    // prints distance 
    Serial.println(distance);
    // if obstacle , stop, basically 
    if (obstacle) {
       Serial.println("*** OBSTACLE DETECTED ***");

        driver.setspeed(0);
        if (SpeedVal>0){
        DrivingLogic(0);
        driver.brake();
        }
        // if speed negative go backwards kinda
        else if(SpeedVal <0){
        DrivingLogic(SpeedVal);
        }
        client.write("Obstacle Detected");
        return;  // Safe now
    }
    if (!obstacle) {
        client.write("No Obstacles Detected");
    }
    // laptop sends START\n buggy goes
    if (cmd == "START\n") { 
        ctrl = true;
        //Serial.println("GUI Says GO!");
    }
    // laptop sends STOP\n buggy stops 
    else if (cmd == "STOP\n") {
        driver.brake();
        ctrl = false ;
       // Serial.println("GUI SAYS STOP!");
    }
    // laptop sends SPEED:(NUM), buggy gets the num and turns it to a speed and then goes at that speed yo
    else if (cmd.startsWith("SPEED:")) {
          GUISpeed = cmd.substring(6).toFloat();  // Parse Speed from string
          SpeedVal = (float(GUISpeed)*0.01);
         // Serial.print("GUI SAYS GO!!! AT : ");
         // Serial.print(GUISpeed);
          //Serial.println("% Speed");
          DrivingLogic(SpeedVal);
    }
}         
    

      
    
    
  

    




    