//Laptop IP 10.94.44.136
//Duino IP 10.94.44.124

import processing.net.*;
import controlP5.*;
import grafica.*;

Chart newChart;
Client client;
ControlP5 cp5;
String status = "Connect to Arduino IP";
int speed;
//int[] numbers = {10, 20 , 30};

float[] reference_speed_array = new float[60];
float[] actual_speed = new float[0];
float[] time = new float[60];
float[] time_stamps = new float[60];
String[] legend_strings = {"Reference", "Buggy Speed"};
//maybe change to a string later depending on if we want more to parse 
//or not
float sse = 0;
String obstacles = "";
String obst_display = "";
String direction_display = "";
String reference_speed = "0";
String current_speed = "0";
String textval = "";
GPlot plot;
int nPoints = 100;
int lastStepTime = 0;
int step = 0;
int count = 0;
int new_count = 0;
int j = 0;
boolean submitted = false;
 
GPointsArray points = new GPointsArray(nPoints);
//switch
ToggleSwitch modeSwitch;

void setup() {
  size(800, 1000);
  background(150);
  PFont font = createFont("arial", 12);

  cp5 = new ControlP5(this);
  int btnW = 150, btnH = 80, btnY = 120;
  for (int i = 0; i < time.length; i++) {
  time[i] = i;
  }

  plot = new GPlot(this, width/9, height - 925, width - 200, 650);
  
  // Set the plot title and the axis labels

  plot.setTitleText("Buggy speed in response to the given profile");
  plot.getXAxis().setAxisLabelText("Time (seconds)");
  plot.getYAxis().setAxisLabelText("Speed cm/s");
  
  plot.setBgColor(50);
  plot.setLabelBgColor(50);
  plot.setAllFontProperties("arial", 255, 16);
  
  plot.setXLim(0, 60);
  plot.setYLim(0, 50);
  
  plot.setLineColor(color(50, 100, 200));
  plot.setLineWidth(2);
  plot.setPointColor(color(50, 100, 200));
  plot.setPointSize(2);
  
  // Speed: -100 reverse → 0 stop → 100 forward
  /*cp5.addSlider("speed")
     .setPosition(width/3.5,height/1.5)
     .setSize(sliderW, sliderH)
     .setRange(-100, 100)
     .setValue(0)
     .setNumberOfTickMarks(23)
     .setTriggerEvent(Slider.RELEASE); 
  */
  
  cp5.addTextfield("Input a command")
     .setPosition(width/4, height-100)
       .setSize(300, 40)
         .setFont(createFont("arial", 12))
           .setAutoClear(false)
             ;

  cp5.addBang("Submit")
    .setPosition((width/4)+300, height-100)
      .setSize(80, 40)
        .setFont(font)
          .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
            ;

  textFont(font);

  
  //client = new Client(this, "172.20.10.9", 5200);  // duino IP
  
  modeSwitch = new ToggleSwitch(
  width/2, height-250,  // position
  80, 30,                   // width, height
  "analogue", "digital",
  false                     // start on analogue
  );
}

void draw() {
  //check for null client
  /*if (client == null || !client.active()) {
    status = "Reconnecting...";
    try {
      client = new Client(this, "172.20.10.9", 5200);
    } catch (Exception e) {
      client = null;  
    }
  }
  */
  plot.setPoints(points);
  
  background(50);
  plot.beginDraw();
  plot.drawBackground();
  plot.drawBox();
  plot.drawXAxis();
  plot.drawYAxis();
  plot.drawTitle();
  plot.drawGridLines(GPlot.BOTH);
  plot.drawPoints();
  plot.drawLines();
  //plot.drawLegend(legend_strings, time, actual_speed);
  plot.endDraw();
  
  
  fill(255);
  textSize(20);
  textAlign(CENTER, CENTER);
  text("Buggy Control", width/2, 40);
  //newChart.push("numbers", (2*sin(frameCount*0.1)*10));
  
   modeSwitch.draw();

  text(status, width/2, height-200 );
  text(obst_display, width/2, height-800);
  text(direction_display, width/2, height-900);
  text("Reference : ", 250, height - 175);
  text(reference_speed, width/2, height - 175);
  text("cm/second", 500, height - 175);
  text("Actual speed : ", 250, height - 150);
  text(current_speed, width/2, height - 150);
  text("cm/second", 500, height - 150);
 
 /*if (client != null && client.active() && client.available() > 0) {  
    String response = client.readStringUntil('\n');
    if (response != null) {
      response = response.trim();
      println("Raw:", response);
      if (response.startsWith("DIST:")) {
        distance_travelled = response.substring(5);
      }
      else if (response.startsWith("SPD:")) {
        current_speed = response.substring(4);
      }
      else if (!response.equals("Hello Client")){
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
  */
  
  if (submitted) {
    for (int i = 0; i < textval.length(); i++) {
      if (textval.charAt(i) == ';')
         count++;
    }
    String[] pairs = textval.split("; ");
    for (int i = 0; i < pairs.length; i++) {
      String[] numbers = pairs[i].split(" ");
      time_stamps[i] = Float.parseFloat(numbers[0]);
      reference_speed_array[i] = Float.parseFloat(numbers[1]);
    }
  }
  
  if (submitted && millis() - lastStepTime > 1000) {
    if (new_count < 60) {
      points.add(new_count, reference_speed_array[0]);
      new_count++;
      if (new_count == time_stamps[j]) {
        j++;
      }
      lastStepTime = millis();
    } else {
      new_count = 0;
      submitted = false;
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
  //client.write(textval + "\n");
  print(" Command = " + textval);
  println();
  submitted = true;

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
