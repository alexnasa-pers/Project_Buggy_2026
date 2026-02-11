#include "driver.h"
#include "TCRT5000.h"
#include "HCSR04.h"
//CHUD-LI CODE!!!
TCRT5000 leftsensor(12);
TCRT5000 rightsensor(2);
HC_SR04 UltraSensor(7,8);
bool obstacle = false;


L293D driver(5, 11, 6, 9, 3, 10);






void setup() {
    Serial.begin(115200);
    driver.begin();
   // attachInterrupt(digitalPinToInterrupt(8), L293D::brakeISR , RISING);
}

void loop() {
    float distance = UltraSensor.centimeters();
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    obstacle = (distance < 10 && distance > 0);  // Update every loop
    
    if (obstacle) {
        Serial.println("*** OBSTACLE DETECTED ***");
        driver.setspeed(0);
        driver.brake();
        return;  // Safe now
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
        driver.setLspeed(0.35);
        driver.setRspeed(0.75);
    }
    if (leftsensor.dark() && rightsensor.bright()){
       // Serial.println("***LEFT DARK***");
        driver.setLspeed(0.75);
        driver.setRspeed(0.35);
    }
    if (leftsensor.bright() && rightsensor.bright()){
        //Serial.println("***ALL BRIGHT***");
        driver.setLspeed(0);
        driver.setRspeed(0.5);
    }
    if (leftsensor.dark() && rightsensor.dark()){
       // Serial.println("***ALL DARK***");
        driver.setspeed(0.8);
      
    }
    
}

    





