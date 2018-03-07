#include "ofApp.h"

using namespace ofxCv;

void ofApp::setup() {
	cam.setup(1280, 720);
	tracker.setup();
}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		tracker.update(toCv(cam));
	}
}

void ofApp::draw() {
    ofSetColor(255);
	cam.draw(0, 0);
	ofSetLineWidth(2);
	tracker.draw();
    ofPolyline noseBase = tracker.getImageFeature(ofxFaceTracker::NOSE_BASE);
    ofSetColor(ofColor::red);
    noseBase.draw();
    ofDrawCircle(noseBase.getCentroid2D(), 8 * tracker.getScale());
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
}

void ofApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}
