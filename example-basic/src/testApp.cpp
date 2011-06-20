#include "testApp.h"

void testApp::setup() {
	ofSetVerticalSync(true);
	ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
	cam.initGrabber(640, 480);
	
	tracker.setup();
	tracker.setScale(.5);
}

void testApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		tracker.update(toCv(cam.getPixelsRef()));
	}
}

void testApp::draw() {
	ofSetColor(255);
	cam.draw(0, 0);
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
	
	ofSetLineWidth(1);
	tracker.draw();
	
	ofPolyline leftEye = tracker.getFeature(ofxFaceTracker::LEFT_EYE);
	ofPolyline rightEye = tracker.getFeature(ofxFaceTracker::RIGHT_EYE);
	
	ofSetLineWidth(2);
	ofSetColor(ofColor::red);
	leftEye.draw();
	ofSetColor(ofColor::green);
	rightEye.draw();
}

void testApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}