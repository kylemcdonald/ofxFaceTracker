#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
	ofSetVerticalSync(true);
	ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
	cam.initGrabber(640, 480);
	
	tracker.setup();
	tracker.setRescale(.5);
}

void testApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		tracker.update(toCv(cam));
	}
}

void testApp::draw() {
	ofSetColor(255);
	cam.draw(0, 0);
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
	
	ofPolyline leftEye = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE);
	ofPolyline rightEye = tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE);
	ofPolyline faceOutline = tracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE);
	
	ofSetLineWidth(2);
	ofSetColor(ofColor::red);
	leftEye.draw();
	ofSetColor(ofColor::green);
	rightEye.draw();
	ofSetColor(ofColor::blue);
	faceOutline.draw();
	
	ofSetLineWidth(1);
	ofSetColor(255);
	tracker.draw();
}

void testApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}