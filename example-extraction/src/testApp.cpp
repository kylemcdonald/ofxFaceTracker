#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
	ofSetVerticalSync(true);
	cam.initGrabber(640, 480);
	tracker.setup();
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
	
	if(tracker.getFound()) {
		tracker.draw();
		ofMesh objectMesh = tracker.getObjectMesh();
		ofMesh meanMesh = tracker.getMeanObjectMesh();
		
		ofSetupScreenOrtho(640, 480, OF_ORIENTATION_DEFAULT, true, -1000, 1000);
		ofTranslate(100, 100);
		ofScale(5,5,5);
		cam.getTextureReference().bind();
		meanMesh.draw();
		cam.getTextureReference().unbind();
	}
}

void testApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}