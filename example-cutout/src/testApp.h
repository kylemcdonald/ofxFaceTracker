#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxFaceTracker.h"
#include "ofxDelaunay.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	ofImage img;
	ofVideoGrabber cam;
	ofxFaceTracker camTracker, imgTracker;
};
