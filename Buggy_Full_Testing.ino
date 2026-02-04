#include "driver.h"
#include "TCRT5000.h"
#include "HCSR04.h"

TCRT5000 LeftSensor(13);
TCRT5000 RightSensor(12);
HC_SR04 UltraSensor(7,8);

L293D driver(5, 11, 6, 9, 3, 10);





void setup() {
    Serial.begin(9600);
    driver.begin();
}

void loop() {
    // float distance =  UltraSensor.centimeters();
    // // Update speeds every loop (critical!)
    // driver.speed(255, 255);
    
    // // Left sensor control
    // if (LeftSensor.dark()) {
    //     driver.leftForward();
    // } else {
    //     driver.leftCoast();
    // }
    
    // // Right sensor control  
    // if (RightSensor.dark()) {
    //     driver.rightForward();
    // } else {
    //     driver.rightCoast();
    // }
    
    // delay(50);  // Stability

    // if (distance < 20 && distance > 0){
    //   driver.brake();
    //   return;
    // }

    float distance = UltraSensor.centimeters();
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    
    if (distance < 20 && distance > 0) {
        Serial.println("*** OBSTACLE DETECTED ***");
        driver.brake();
    } else {
        driver.forward();
    }
    
    delay(100);
}



