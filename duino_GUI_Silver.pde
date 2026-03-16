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

//alexs variable that he put in (into the code) (11/03/2026) (larping dont stop)
String textval = "";
//switch
ToggleSwitch modeSwitch;

void setup() {
  size(800, 600);

  PFont font = createFont("arial", 12);

  cp5 = new ControlP5(this);
  int btnW = 150, btnH = 80, btnY = 120;
  // Big green START button
  cp5.addButton("startBuggy")
     .setCaptionLabel("START")
     .setPosition(((width - 2.25*btnW)/2), ((height/3)-btnY))
     .setSize(150, 80)
     .setColorBackground(color(0, 200, 0))
     .setColorForeground(color(0, 255, 0))
     .setColorActive(color(0, 150, 0));
  
  // Big red STOP button
  cp5.addButton("stopBuggy")
     .setCaptionLabel("STOP")
     .setPosition((width + btnW/4)/2, ((height/3)-(btnY)))
     .setSize(150, 80)
     .setColorBackground(color(200, 0, 0))
     .setColorForeground(color(255, 0, 0))
     .setColorActive(color(150, 0, 0));
  int centerX = width / 2;
  int sliderW = 300, sliderH = 40;
  
  // Speed: -100 reverse → 0 stop → 100 forward
  cp5.addSlider("speed")
     .setPosition(width/3.5,height/1.5)
     .setSize(sliderW, sliderH)
     .setRange(-100, 100)
     .setValue(0)
     .setNumberOfTickMarks(23)
     .setTriggerEvent(Slider.RELEASE); 

  cp5.addTextfield("Input a command")
     .setPosition(width/4, height/1.25)
       .setSize(300, 40)
         .setFont(createFont("arial", 12))
           .setAutoClear(false)
             ;

  cp5.addBang("Submit")
    .setPosition((width/4)+300, height/1.25)
      .setSize(80, 40)
        .setFont(font)
          .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
            ;

  textFont(font);

  
  client = new Client(this, "10.94.44.124", 5200);  // duino IP
  
  modeSwitch = new ToggleSwitch(
  width/2, height/1.8,  // position
  80, 30,                   // width, height
  "analogue", "digital",
  false                     // start on analogue
  );
}

void draw() {
  //check for null client
  if (client == null || !client.active()) {
    status = "Reconnecting...";
    try {
      client = new Client(this, "10.105.204.124", 5200);
    } catch (Exception e) {
      client = null;  
    }
  }

  background(50);
  fill(255);
  textSize(20);
  textAlign(CENTER, CENTER);
  text("Buggy Control", width/2, 40);
  
   modeSwitch.draw();

  text(status, width/2, height/2 );
  text(obst_display, width/2, height/2.5);
  text(direction_display, width/2, height/2.75);
 
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
      else if(!response.equals("Hello Client") && (response.equals("Going Forward") || response.equals("Turning Left") || response.equals("Turning Right") || response.equals("Circling"))){
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

void Submit() {
  textval = cp5.get(Textfield.class, "Input a command").getText();
  client.write("Command: " + textval + "\n");
  print(" Command = " + textval);
  println();

  cp5.get(Textfield.class, "Input a command").clear();
}

void mousePressed() {
  if (modeSwitch != null) {
    modeSwitch.mousePressed();
  }
}

class ToggleSwitch {
  float x, y, w, h;
  String leftLabel, rightLabel;
  boolean on;

  ToggleSwitch(float x, float y, float w, float h,
               String leftLabel, String rightLabel,
               boolean initialState) {
    this.x = x;
    this.y = y;
    this.w = w;
    this.h = h;
    this.leftLabel = leftLabel;
    this.rightLabel = rightLabel;
    this.on = initialState;
  }

  void draw() {
    pushStyle();
    // labels
    fill(255);
    textAlign(CENTER, CENTER);
    text(leftLabel,  x - (w+20), y);
    text(rightLabel, x + w, y);

    // track
    noStroke();
    fill(120);
    rectMode(CENTER);
    rect(x, y, w, h, h/2);

    // thumb
    float thumbX = on ? x + w/4 : x - w/4;
    fill(on ? color(0, 200, 0) : color(200, 0, 0));
    ellipse(thumbX, y, h-4, h-4);
    
    popStyle();
  }

  void mousePressed() {
    if (mouseX > x - w/2 && mouseX < x + w/2 &&
        mouseY > y - h/2 && mouseY < y + h/2) {
      on = !on;
     
    //send to arduino
    if (client != null && client.active()) {
        String mode = on ? "digital" : "analogue";
        client.write("MODE:" + mode + "\n");
        println("Sent MODE:" + mode);
      }
    }
  }

  boolean isOn() {
    return on;
  }
}
