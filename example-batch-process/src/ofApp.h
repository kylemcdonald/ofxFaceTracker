#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "ofxFaceTracker.h"
#include "ofxTiming.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
	void draw();
    void saveFace(), loadFace();
    
    ofDirectory dir;
    int index;
	ofxFaceTracker tracker;
    int iterations;
    ofImage img;
    ofMesh prevFace;
    RateTimer timer;
};
