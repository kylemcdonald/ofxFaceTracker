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
    
	ofVideoPlayer video;
	ofxFaceTracker tracker;
    vector<ofMesh> trackedFrames;
    vector<vector<float>> trackedGestures;

};
