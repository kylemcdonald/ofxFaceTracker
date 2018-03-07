#include "ofApp.h"

using namespace ofxCv;

void ofApp::setup() {
	cam.setup(1280, 720);
    ofBackground(0);
	tracker.setup();
}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		tracker.update(toCv(cam));
	}
}

void ofApp::draw() {
    if(ofGetKeyPressed()) {
        ofSetColor(255);
        cam.draw(0, 0);
        ofSetColor(255, 32);
        ofSetLineWidth(1);
        tracker.getImageMesh().drawWireframe();
        tracker.draw(true);
    }
    ofSetColor(255);
    ofSetLineWidth(2);
    ofPushStyle();
    ofNoFill();
    ofSetColor(255);
    overlay.draw(tracker);
    ofPopStyle();
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
}

void ofApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}
