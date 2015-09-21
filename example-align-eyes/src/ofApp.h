#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "ofxFaceTracker.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	ofxFaceTracker tracker;
    ofDirectory dir;
    ofImage img;
    int cur;
    float eyeDistance;
    ofFbo fbo;
    ofPixels pix;
};
