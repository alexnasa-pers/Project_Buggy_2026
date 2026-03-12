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
char ssid[] = "Alexiiphone"; // your network SSID
char pass[] = "alexboiq"; // your network password
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
//distance to travel

//Driver class constructor 
L293D driver(5, 11, 6, 9, 3, 10);
//Opens a server on port 5200
WiFiServer server(5200);
WiFiClient client;
//obstacle detection send timing
unsigned long lastStatusTime = 0;
const unsigned long statusInterval = 300;
bool sendObst = false;
int direction = 0; //1=forward , 2=left , 3= right 4= stopped 5 = circling 
int last_direction = 0 ;

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
    // sets up wifi client so laptop can connect
    if (!client){client = server.available ();}
    

  if (client) {   
    //if(sendObst){ Serial.println("Client Connected") ;  }  // a client has connected
                
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
            Serial.println("Imagine chud lee is going forward");
            break;
          case 'B':
            //tbc 
            client.print("Going Backwards");
            Serial.print("Going Backwards to distance ");
            Serial.print(parameter);
            Serial.println("cm");
            Serial.println("(Imagine he go backwards)");
            break;
          case 'R':
            //Turn right by x degrees
            client.print("Going Right");
            Serial.print("Turning right at angle ");
            Serial.print(parameter);
            Serial.println(" degrees");
            break;
          case 'L':
            //turn left by x degrees
            client.println("Turning Left");
            Serial.print("Turning left at angle ");
            Serial.print(parameter);
            Serial.println(" degrees");
            break;
        }
        
    }
  }
    //stores distance from  ultrasonic 
    float distance = UltraSensor.centimeters();
    // Serial.print("Distance: ");
    // Serial.print(distance);
    // Serial.println(" cm");
    // boolean for obstacle detection 
    obstacle = (distance < 20 && distance > 0);  // Update every loop
    // prints distance 
    //Serial.println(distance);
    // if obstacle , stop, basically 
    unsigned long now = millis();
    if (now - lastStatusTime >= statusInterval){
        lastStatusTime = now;
        sendObst = true;
    }
    else {
        sendObst = false;
    }
    if (obstacle) {
        
        
        if(sendObst){
            client.println("Obstacle Detected\n");
           // Serial.println("*** OBSTACLE DETECTED ***");
            }
        
        
        driver.brake();
        direction = 0;
        ctrl = false ;
       // return; 
        }
  
         // Safe now
    else  if (!obstacle) {
        ctrl = true;
    if(sendObst){
    client.println("No Obstacles Detected\n");
   //  Serial.println("No obst");}
    }
    
    }
}


       
    

      
    
    
  

    




    
