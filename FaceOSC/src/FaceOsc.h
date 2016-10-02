#pragma once

#include "ofxFaceTracker.h"
#include "ofxOsc.h"

class FaceOsc {
public:
	void clearBundle();
	void addMessage(string address, ofVec3f data);
	void addMessage(string address, ofVec2f data);
	void addMessage(string address, float data);
	void addMessage(string address, int data);
	void sendBundle();
    
    void sendFaceOsc(ofxFaceTracker& tracker);
    
	string host;
	int port;
	ofxOscSender osc;
	ofxOscBundle bundle;
    
    ofParameter<bool> bIncludePose;
    ofParameter<bool> bIncludeGestures;
    ofParameter<bool> bIncludeAllVertices;
    ofParameter<bool> bNormalizeRaw;
};