//Laptop IP 10.212.132.136
//Duino IP 10.212.132.100

import processing.net.*;
import controlP5.*;

Client client;
ControlP5 cp5;
String status = "Connect to Arduino IP";
int speed;

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
     .setNumberOfTickMarks(11);

  
  client = new Client(this, "10.212.132.100", 1234);  // duino IP
}

void draw() {
   if (!client.active()) {
    status = "Connection failed - check IP/WiFi";
  }
  if (millis() % 5000 == 0) {  // Periodic ping
  if (client != null && client.active()) client.write("PING\n");
  }
  background(50);
  fill(255);
  textSize(20);
  textAlign(CENTER,CENTER);
  text("Buggy Control", width/2, 40);
  text(status, width/2, (height/2));
  
  // Check Arduino response
  if (client.available() > 0) {
    status = client.readStringUntil('\n');
    if (status != null) status = status.trim();
  }
}

void startBuggy() {
  if (client != null) {
    client.write("START\n");
    status = "Sent START...";
  }
}

void stopBuggy() {
  if (client != null) {
    client.write("STOP\n");
    status = "Sent STOP...";
  }
}


void speed(int val) {  // ADD THIS
  speed = val;
  if (client != null) {
    client.write("SPEED:" + (int)val + "\n");
    status = "Speed: " + (int)val;
  }
}
