//Laptop IP 10.94.44.136
//Duino IP 

import processing.net.*;
import controlP5.*;
import grafica.*;

Chart newChart;
Client client;
ControlP5 cp5;
String status = "Connect to Arduino IP";

//important arrays
float[] reference_speed_array = new float[60];
float actual_speed = 0;
float actual_time = 0;
float start_time = 0;
float[] time_stamps = new float[60];
String[] legend_strings = {"Reference", "Buggy Speed"};

//Strings for things 
String reference_speed = "0";
String current_speed = "0";
String textval = "";

//misc floats
float sse = 0;
float sum_of_stamps_so_far = 0;
int lastStepTime = 0;
int step = 0;
int count = 0;
int numPoints = 0;
int new_count = 0;
int which_duration = 0;
boolean submitted = false;
float sum = 0;
int currentPointIndex = 0;
float totalTimePoints = 0;

//chart points
GPointsArray points;
GPointsArray points2;
GPlot plot;

void setup() {
  size(800, 1000);
  background(150);
  PFont font = createFont("arial", 12);
  
  plot = new GPlot(this, width/9, height - 925, width - 200, 650);
  cp5 = new ControlP5(this);
  points = new GPointsArray();
  points2 = new GPointsArray();
  int btnW = 150, btnH = 80, btnY = 120;
  
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
  
  plot.addLayer("speed", points2);
  plot.getLayer("speed").setLineColor(color(200, 50, 50));
  plot.getLayer("speed").setPointColor(color(200, 50, 50));
  plot.getLayer("speed").setLineWidth(2);
  plot.getLayer("speed").setPointSize(2);
  
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
  
  client = new Client(this, "172.20.10.9", 5200);  // duino IP
  
  plot.setPoints(points);
}

void draw() {
  //check for null client
  if (client == null || !client.active()) {
    status = "Reconnecting...";
    try {
      client = new Client(this, "172.20.10.9", 5200);
    } catch (Exception e) {
      client = null;  
    }
  }
  //set points
  plot.setPoints(points);
  plot.getLayer("speed").setPoints(points2);
  
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
  //REMINDER: MAKE LEGEND WORK
  //plot.drawLegend(legend_strings, time, actual_speed);
  plot.endDraw();
  
  if (client != null && client.active() && client.available() > 0) {
    
    String response = client.readStringUntil('\n');
    
    if (response != null) {
      response = response.trim();
      println("Raw:", response);
      if (response.startsWith("SPD: ")) {
        try {
          actual_speed = Float.parseFloat(response.substring(5));
          actual_time = (millis() - start_time) / 1000;
          points2.add(actual_time, actual_speed);
        } catch (NumberFormatException e) {
          println("Skipping malformed SPD line: " + response);
        }
      }
      if (response.startsWith("MSE: ")) {
         try {
          current_speed = response.substring(5);
          Float.parseFloat(current_speed); // validate it's actually a number
        } catch (NumberFormatException e) {
          println("Skipping malformed MSE line: " + response);
        }
      }
    } 
    
  }
  if (submitted) {
    points = new GPointsArray();
    
    float t = 0;
    for (int i = 0; i < numPoints; i++) {
      float t_end = t + time_stamps[i];
      
      if (i > 0) points.add(t, reference_speed_array[i]);  
      points.add(t,     reference_speed_array[i]);          
      points.add(t_end, reference_speed_array[i]); 
      
      t = t_end;
    }
    
    plot.setPoints(points);
    submitted = false;
  }
  /*
  if (submitted && millis() - lastStepTime > 1000) {
    if (currentPointIndex < time_stamps[which_duration+1]) {
      points.add(currentPointIndex, reference_speed_array[which_duration]);
      currentPointIndex++;
      lastStepTime = millis();
      if (currentPointIndex == sum_of_stamps_so_far) {
        sum_of_stamps_so_far += time_stamps[which_duration];
        which_duration++;
        println("Index of timestamps: " + which_duration);
      }
    } else {
        submitted = false;
        currentPointIndex = 0;
        which_duration = 0;
    }
    
  }
  */
  fill(255);
  textSize(20);
  textAlign(CENTER, CENTER);
  text("Buggy Control", width/2, 40);
 
  text(status, width/2, height-200 );
  
  text("Reference : ", 250, height - 175);
  text(reference_speed, width/2, height - 175);
  text("cm/second", 500, height - 175);
  text("MSE : ", 250, height - 150);
  text(current_speed, width/2, height - 150);
}

void Submit() {
  //Prints the thing at the bottom and gets the string
  textval = cp5.get(Textfield.class, "Input a command").getText();
  print(" Command = " + textval);
  client.write(textval);
  println();
  
  String[] pairs = textval.split(": ");
  numPoints = 0;
  
  for (int i = 0; i < pairs.length; i++) {
    String[] numbers = pairs[i].split(" ");
    if (numbers.length >= 2)
       numPoints++;
  }
    
  time_stamps = new float[numPoints];
  reference_speed_array = new float[numPoints];
  
  for (int i = 0; i < pairs.length; i++) {
    String[] numbers = pairs[i].split(" ");
    if (numbers.length >= 2) {
      time_stamps[i] = Float.parseFloat(numbers[0]);
      reference_speed_array[i] = Float.parseFloat(numbers[1]);
    }
  }
  
  if (numPoints > 0) {
    for (int i = 0; i < time_stamps.length; i++) {
      totalTimePoints += time_stamps[i];
    }
    sum_of_stamps_so_far = time_stamps[0];
  }
  
  
  points = new GPointsArray();
  points2 = new GPointsArray();
  plot.setPoints(points);
  plot.getLayer("speed").setPoints(points2);
  
  
  submitted = true;
  lastStepTime = millis();
  
  cp5.get(Textfield.class, "Input a command").clear();
  start_time = millis();
}
