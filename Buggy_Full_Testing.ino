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

int status = WL_IDLE_STATUS;

//Speed control over GUI
int GUISpeed = 0;
float SpeedVal = 0.0;
//checking the ping
bool CheckedPing = false;
bool ctrl = false;
bool said_hello = false;
String cmd = "";
L293D driver(5, 11, 6, 9, 3, 10);
WiFiServer server(5200);
String msg = "";
void DrivingLogic(){
    if (leftsensor.bright() && rightsensor.dark()){
       Serial.println("***RIGHT DARK***");
        driver.setLspeed(0.35);
        driver.setRspeed(0.75);
    }
    if (leftsensor.dark() && rightsensor.bright()){
       Serial.println("***LEFT DARK***");
        driver.setLspeed(0.75);
        driver.setRspeed(0.35);
    }
    if (leftsensor.bright() && rightsensor.bright()){
        Serial.println("***ALL BRIGHT***");
        driver.setLspeed(0);
        driver.setRspeed(0.5);
    }
    if (leftsensor.dark() && rightsensor.dark()){
       Serial.println("***ALL DARK***");
        driver.setspeed(0.8);
}
}

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
    Serial.begin(115200);
    delay(2000);
    Serial.println("Booted");
    driver.begin();
    while (status != 3) { // Attempt to connect to AP
        Serial.print("Attempting to connect to Network named:␣");
        Serial.println(ssid);
        Serial.println(status);
        status = WiFi.begin(ssid, pass);
        delay(1500); // give it time to connect
    }
    IPAddress ip = WiFi.localIP();
    Serial.print("IP␣Address:␣"); 
    Serial.println(ip);
    server.begin();


}

void loop() {
    if (ctrl){
        DrivingLogic(SpeedVal);
    }
        
    WiFiClient client = server.available ();
    if ( client.connected ()) {
        if (!said_hello){
        client.write("Hello Client\n");
        }
        cmd = client.readString();
        if (cmd != ""){
            Serial.println(cmd);
            //if (cmd == "START\n"){
               // Serial.println("im gonna go");
            }
        }
    

    float distance = UltraSensor.centimeters();
    // Serial.print("Distance: ");
    // Serial.print(distance);
    // Serial.println(" cm");

    obstacle = (distance < 10 && distance > 0);  // Update every loop
    Serial.println(distance);

    if (obstacle) {
       Serial.println("*** OBSTACLE DETECTED ***");

        driver.setspeed(0);
        if (SpeedVal>0){
        DrivingLogic(0);
        driver.brake();
        }
        else if(SpeedVal <0){
        DrivingLogic(SpeedVal);
        }
        client.write("Obstacle Detected");
        return;  // Safe now
    }
    if (!obstacle) {
        client.write("No Obstacles Detected");
    }

    if (cmd == "START\n") { 
        ctrl = true;
        //Serial.println("GUI Says GO!");
    }
    else if (cmd == "STOP\n") {
        driver.brake();
        ctrl = false ;
       // Serial.println("GUI SAYS STOP!");
    }
    else if (cmd.startsWith("SPEED:")) {
          GUISpeed = cmd.substring(6).toFloat();  // Parse Speed from string
          SpeedVal = (float(GUISpeed)*0.01);
         // Serial.print("GUI SAYS GO!!! AT : ");
         // Serial.print(GUISpeed);
          //Serial.println("% Speed");
          DrivingLogic(SpeedVal);
    }
}         
    

      
    
    
  

    




    