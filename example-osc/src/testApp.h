#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "ofxFaceTracker.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

class testApp : public ofBaseApp {
public:
	void loadSettings();

	void clearBundle();
	template <class T>
	void addMessage(string address, T data);
	void sendBundle();

	void setup();
	void update();
	void draw();
	void keyPressed(int key);

	int camWidth, camHeight;

	string host;
	int port;
	ofxOscSender osc;
	ofxOscBundle bundle;

	ofVideoGrabber cam;
	ofxFaceTracker tracker;
	float scale;
	ofMatrix4x4 rotationMatrix;
};
