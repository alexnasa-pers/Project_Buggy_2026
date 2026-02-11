#include "driver.h"
#include "TCRT5000.h"
#include "HCSR04.h"
#include "WiFiS3.h"



//CHUD-LI CODE!!!
//IR sensor constructors
TCRT5000 leftsensor(12);
TCRT5000 rightsensor(2);
//Ultrasonic Sensor constructor
HC_SR04 UltraSensor(7,8);
//obstacle detection state boolean
bool obstacle = false;
//Network info 
char ssid[] = "Jamie's Pixel 8 Pro"; // your network SSID
char pass[] = "GimmeDatBuggyUggy"; // your network password

int status = WL_IDLE_STATUS;

//Speed control over GUI
int GUISpeed = 0;
float SpeedVal = 0.0;
//checking the ping
bool CheckedPing = false;
String req = "";
L293D driver(5, 11, 6, 9, 3, 10);
WiFiServer server(5200);
String msg = "";




void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Booted");
    driver.begin();
    while (status != WL_CONNECTED) { // Attempt to connect to AP
        Serial.print("Attempting␣to␣connect␣to␣Network␣named:␣");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass);
        delay(10000); // give it time to connect
    }
    IPAddress ip = WiFi.localIP();
    Serial.print("IP␣Address:␣"); 
    Serial.println(ip);
    server.begin();
}

void loop() {

    WiFiClient client = server.available ();
    if ( client.connected () ) {
        client.write("Hello␣Client\n");
        msg = client.readString();
        Serial.println(msg);
    }
  // Always check for clients (independent of ping!)

    float distance = UltraSensor.centimeters();
    // Serial.print("Distance: ");
    // Serial.print(distance);
    // Serial.println(" cm");

    obstacle = (distance < 10 && distance > 0);  // Update every loop
    
    if (obstacle) {
        Serial.println("*** OBSTACLE DETECTED ***");
        driver.setspeed(0);
        driver.brake();
        return;  // Safe now
    }

    if (req == "START") { 
        driver.setspeed(0.8);
 
        Serial.println("GUI Says GO!");
    }
    else if (req == "STOP") {
        driver.brake(); 
        Serial.println("GUI SAYS STOP!");
    }
    else if (req.startsWith("SPEED:")) {
          GUISpeed = req.substring(6).toInt();  // Parse Speed from string
          SpeedVal = (float(GUISpeed)*0.01);
          Serial.print("GUI SAYS GO!!! AT : ");
          Serial.print(GUISpeed);
          Serial.println("% Speed");
          driver.atGUIspeed(SpeedVal);
    }
          
    
    // if (leftsensor.bright()){
    //     driver.left_norm();
    //     delay(500);
    //     Serial.println("***LEFT BRIGHT***");
    // }
    // else if (leftsensor.dark()){
    //     driver.leftBrake();
    //     delay(500);
    //     Serial.println("***LEFT DARK***");
    // }
    // if (rightsensor.bright()){
    //     driver.right_norm();
    //     delay(500);
    //     Serial.println("***RIGHT BRIGHT***");
    // }
    // else if (rightsensor.dark()){
    //     driver.rightBrake();
    //     delay(500);
    //     Serial.println("***RIGHT DARK***");
    // }
    driver.setspeed(0);
    if (leftsensor.bright() && rightsensor.dark()){
       // Serial.println("***RIGHT DARK***");
        driver.setLspeed(0.35*SpeedVal);
        driver.setRspeed(0.75*SpeedVal);
    }
    if (leftsensor.dark() && rightsensor.bright()){
       // Serial.println("***LEFT DARK***");
        driver.setLspeed(0.75*SpeedVal);
        driver.setRspeed(0.35*SpeedVal);
    }
    if (leftsensor.bright() && rightsensor.bright()){
        //Serial.println("***ALL BRIGHT***");
        driver.setLspeed(0*SpeedVal);
        driver.setRspeed(0.5*SpeedVal);
    }
    if (leftsensor.dark() && rightsensor.dark()){
       // Serial.println("***ALL DARK***");
        driver.setspeed(0.8*SpeedVal);
      
    }
    }
  

    




    




