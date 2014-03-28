#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "FaceOsc.h"
#include "ofxXmlSettings.h"

class testApp : public ofBaseApp, public FaceOsc {
public:
	void loadSettings();

	void setup();
	void update();
	void draw();
	void keyPressed(int key);

	void setVideoSource(bool useCamera);

	bool bUseCamera, bPaused;

	int camWidth, camHeight;
	int movieWidth, movieHeight;
	int sourceWidth, sourceHeight;

	ofVideoGrabber cam;
	ofVideoPlayer movie;
	ofBaseVideoDraws *videoSource;
    
	ofxFaceTracker tracker;
	ofMatrix4x4 rotationMatrix;
	
	bool bDrawMesh;
};
