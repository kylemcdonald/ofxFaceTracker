import oscP5.*;
OscP5 oscP5;

boolean bCurrentBlinkState; 

//-----------------------------------
void setup() {
  size(640, 480);
  
  oscP5 = new OscP5(this, 8338);
  oscP5.plug(this, "blinkDataReceived", "/gesture/blink");
  bCurrentBlinkState = false;
}

//-----------------------------------
void draw() { 
  if (bCurrentBlinkState == true){
    background(255,0,0);
    fill(255); 
    text("BLINK!", 20,20); 
  } else {
    background(0);
    fill(255); 
    text("No blink.", 20,20); 
  }
}

//-----------------------------------
public void blinkDataReceived(int value) {
  println("Blink Data: " + value);
  bCurrentBlinkState = ((value == 1) ? true : false);
}

//-----------------------------------
// all other OSC messages end up here
void oscEvent(OscMessage m) {
  if (m.isPlugged() == false) {
    println("UNPLUGGED: " + m);
  }
}