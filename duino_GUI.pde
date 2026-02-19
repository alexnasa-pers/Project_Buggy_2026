//Laptop IP 10.94.44.136
//Duino IP 10.94.44.124

import processing.net.*;
import controlP5.*;

Client client;
ControlP5 cp5;
String status = "Connect to Arduino IP";
int speed;
String obstacles = "";
String obst_display = "";
String direction_display = "";
void setup() {
  size(500, 400);
  cp5 = new ControlP5(this);
  int btnW = 150, btnH = 80, btnY = 120;
  // Big green START button
  cp5.addButton("startBuggy")
     .setCaptionLabel("START")
     .setPosition(((width - 2.25*btnW)/2), ((height/2)-btnY))
     .setSize(150, 80)
     .setColorBackground(color(0, 200, 0))
     .setColorForeground(color(0, 255, 0))
     .setColorActive(color(0, 150, 0));
  
  // Big red STOP button
  cp5.addButton("stopBuggy")
     .setCaptionLabel("STOP")
     .setPosition((width + btnW/4)/2, ((height/2)-(btnY)))
     .setSize(150, 80)
     .setColorBackground(color(200, 0, 0))
     .setColorForeground(color(255, 0, 0))
     .setColorActive(color(150, 0, 0));
  int centerX = width / 2;
  int sliderW = 300, sliderH = 40;
  
  // Speed: -100 reverse → 0 stop → 100 forward
  cp5.addSlider("speed")
     .setPosition(100,300)
     .setSize(sliderW, sliderH)
     .setRange(-100, 100)
     .setValue(0)
     .setNumberOfTickMarks(11)
     .setTriggerEvent(Slider.RELEASE); 

  
  client = new Client(this, "10.94.44.124", 5200);  // duino IP
}

void draw() {
  //check for null client
  if (client == null || !client.active()) {
    status = "Reconnecting...";
    try {
      client = new Client(this, "10.94.44.124", 5200);
    } catch (Exception e) {
      client = null;  
    }
  }

  background(50);
  fill(255);
  textSize(20);
  textAlign(CENTER, CENTER);
  text("Buggy Control", width/2, 40);
  
  

  text(status, width/2, height/2);
  text(obst_display, width/2, height/1.5);
  text(direction_display, width/2, height/1.75);
 
 if (client != null && client.active() && client.available() > 0) {  
    String response = client.readStringUntil('\n');                   
    if (response != null) {
      response = response.trim();
      println("Raw:", response);

      if (!response.equals("Hello Client")){
        if ((response.equals("Obstacle Detected") ||
           response.equals("No Obstacles Detected"))) {
               obst_display = response;   // update every time we get a message
        if (obst_display.equals("Obstacle Detected")){
          direction_display = "Stopped";
        }
           }
      else if(!response.equals("Hello Client") && (response.equals("Going Forward") || response.equals("Going Left") || response.equals("Going Right") || response.equals("Circling"))){
          direction_display = response;
        }
      }
    }
  }
}

void startBuggy() {
  if (client != null && client.active()) {
    client.write("START\n");
    status = "Sent START...";
  }
}

void stopBuggy() {
  if (client != null && client.active()) {
    client.write("STOP\n");
    status = "Sent STOP...";
  }
  direction_display = "Stopped";
}


void speed(int val) {  // ADD THIS
  speed = val;
  if (client.active()) {
    client.write("SPEED:" + (int)val + "\n");
    status = "Speed: " + (int)val;
  }
}
