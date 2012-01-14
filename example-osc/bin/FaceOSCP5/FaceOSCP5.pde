// This example is courtesy of Greg Borenstein:
// http://www.gregborenstein.com/
// https://gist.github.com/1603230

import oscP5.*;
OscP5 oscP5;

PVector posePosition;
boolean found;
float eyeLeftHeight;
float eyeRightHeight;
float mouthHeight;
float mouthWidth;
float nostrilHeight;
float leftEyebrowHeight;
float rightEyebrowHeight;

float poseScale;

void setup() {
  size(640, 480);
  frameRate(30);

  oscP5 = new OscP5(this, 8338);
  oscP5.plug(this, "mouthWidthReceived", "/gesture/mouth/width");
  oscP5.plug(this, "mouthHeightReceived", "/gesture/mouth/height");
  oscP5.plug(this, "eyebrowLeftReceived", "/gesture/eyebrow/left");
  oscP5.plug(this, "eyebrowRightReceived", "/gesture/eyebrow/right");
  oscP5.plug(this, "eyeLeftReceived", "/gesture/eye/left");
  oscP5.plug(this, "eyeRightReceived", "/gesture/eye/right");
  oscP5.plug(this, "jawReceived", "/gesture/jaw");
  oscP5.plug(this, "nostrilsReceived", "/gesture/nostrils");
  oscP5.plug(this, "found", "/found");
  oscP5.plug(this, "poseOrientation", "/pose/orientation");
  oscP5.plug(this, "posePosition", "/pose/position");
  oscP5.plug(this, "poseScale", "/pose/scale");
}

void draw() {  
  background(255);
  stroke(0);
  if (found) {

    translate(posePosition.x, posePosition.y);
    scale(poseScale);
    noFill();
    // ellipse(0,0, 3,3);
    ellipse(-20, eyeLeftHeight * -9, 20, 7);
    ellipse(20, eyeRightHeight * -9, 20, 7);
    ellipse(0, 20, mouthWidth* 3, mouthHeight * 3);
    ellipse(-5, nostrilHeight * -1, 7, 3);
    ellipse(5, nostrilHeight * -1, 7, 3);
    rectMode(CENTER);
    fill(0);
    rect(-20, leftEyebrowHeight * -5, 25, 5);
    rect(20, rightEyebrowHeight * -5, 25, 5);
  }
}

public void mouthWidthReceived(float w) {
  println("mouth Width: " + w);
  mouthWidth = w;
}

public void mouthHeightReceived(float h) {
  println("mouth height: " + h);
  mouthHeight = h;
}

public void eyebrowLeftReceived(float h) {
  println("eyebrow left: " + h);
  leftEyebrowHeight = h;
}

public void eyebrowRightReceived(float h) {
  println("eyebrow right: " + h);
  rightEyebrowHeight = h;
}

public void eyeLeftReceived(float h) {
  println("eye left: " + h);
  eyeLeftHeight = h;
}

public void eyeRightReceived(float h) {
  println("eye right: " + h);
  eyeRightHeight = h;
}

public void jawReceived(float h) {
  println("jaw: " + h);
}

public void nostrilsReceived(float h) {
  println("nostrils: " + h);
  nostrilHeight = h;
}

public void found(int i) {
  println("found: " + i); // 1 == found, 0 == not found
  found = i == 1;
}

public void posePosition(float x, float y) {
  println("pose position\tX: " + x + " Y: " + y );
  posePosition = new PVector(x, y);
}

public void poseScale(float s) {
  println("scale: " + s);
  poseScale = s;
}

public void poseOrientation(float x, float y, float z) {
  println("pose orientation\tX: " + x + " Y: " + y + " Z: " + z);
}


void oscEvent(OscMessage theOscMessage) {
  if (theOscMessage.isPlugged()==false) {
    println("UNPLUGGED: " + theOscMessage);
  }
}
