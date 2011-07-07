#include "testApp.h"

void testApp::loadSettings() {
	ofxXmlSettings xml;
	xml.loadFile("settings.xml");
	
	xml.pushTag("camera");
	if(xml.getNumTags("device") > 0) {
		cam.setDeviceID(xml.getValue("device", 0));
	}
	if(xml.getNumTags("framerate") > 0) {
		cam.setDesiredFrameRate(xml.getValue("framerate", 30));
	}
	camWidth = xml.getValue("width", 640);
	camHeight = xml.getValue("height", 480);
	cam.initGrabber(camWidth, camHeight);
	ofSetWindowShape(camWidth, camHeight);
	xml.popTag();
	
	xml.pushTag("face");
	if(xml.getNumTags("scale")) {
		tracker.setScale(xml.getValue("scale", 1.));
	}
	if(xml.getNumTags("iterations")) {
		tracker.setIterations(xml.getValue("iterations", 5));
	}
	if(xml.getNumTags("clamp")) {
		tracker.setClamp(xml.getValue("clamp", 3.));
	}
	if(xml.getNumTags("tolerance")) {
		tracker.setTolerance(xml.getValue("tolerance", .01));
	}
	if(xml.getNumTags("attempts")) {
		tracker.setAttempts(xml.getValue("attempts", 1));
	}
	tracker.setup();
	xml.popTag();
	
	xml.pushTag("osc");
	host = xml.getValue("host", "localhost");
	port = xml.getValue("port", 8338);
	osc.setup(host, port);
	xml.popTag();
}

void testApp::setup() {
	ofSetVerticalSync(true);
	ofSetDataPathRoot("../data/");
	loadSettings();
}

void testApp::clearBundle() {
	bundle.clear();
}

template <>
void testApp::addMessage(string address, ofVec3f data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addFloatArg(data.x);
	msg.addFloatArg(data.y);
	msg.addFloatArg(data.z);
	bundle.addMessage(msg);
}

template <>
void testApp::addMessage(string address, ofVec2f data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addFloatArg(data.x);
	msg.addFloatArg(data.y);
	bundle.addMessage(msg);
}

template <>
void testApp::addMessage(string address, float data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addFloatArg(data);
	bundle.addMessage(msg);
}

template <>
void testApp::addMessage(string address, int data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addIntArg(data);
	bundle.addMessage(msg);
}

void testApp::sendBundle() {
	osc.sendBundle(bundle);
}

void testApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		tracker.update(toCv(cam.getPixelsRef()));
		
		clearBundle();
		
		if(tracker.getFound()) {
			addMessage("/found", 1);
			
			ofVec2f position = tracker.getPosition();
			addMessage("/pose/position", position);
			scale = tracker.getScale();
			addMessage("/pose/scale", scale);
			ofVec3f orientation = tracker.getOrientation();
			addMessage("/pose/orientation", orientation);
			
			addMessage("/gesture/mouth/width", tracker.getGesture(ofxFaceTracker::MOUTH_WIDTH));
			addMessage("/gesture/mouth/height", tracker.getGesture(ofxFaceTracker::MOUTH_HEIGHT));
			addMessage("/gesture/eyebrow/left", tracker.getGesture(ofxFaceTracker::LEFT_EYEBROW_HEIGHT));
			addMessage("/gesture/eyebrow/right", tracker.getGesture(ofxFaceTracker::RIGHT_EYEBROW_HEIGHT));
			addMessage("/gesture/eye/left", tracker.getGesture(ofxFaceTracker::LEFT_EYE_OPENNESS));
			addMessage("/gesture/eye/right", tracker.getGesture(ofxFaceTracker::RIGHT_EYE_OPENNESS));
			addMessage("/gesture/jaw", tracker.getGesture(ofxFaceTracker::JAW_OPENNESS));
			addMessage("/gesture/nostrils", tracker.getGesture(ofxFaceTracker::NOSTRIL_FLARE));
		} else {
			addMessage("/found", 0);
		}
		
		sendBundle();
		
		rotationMatrix = tracker.getRotationMatrix();
	}
}

void testApp::draw() {
	ofSetColor(255);
	cam.draw(0, 0);
	
	if(tracker.getFound()) {
		ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
	
		ofSetLineWidth(1);
		//tracker.draw();
		tracker.getImageMesh().drawWireframe();
		
		ofSetupScreenOrtho(camWidth, camHeight, OF_ORIENTATION_UNKNOWN, true, -1000, 1000);
		ofVec2f pos = tracker.getPosition();
		ofTranslate(pos.x, pos.y);
		applyMatrix(rotationMatrix);
		ofScale(10,10,10);
		ofDrawAxis(scale);
	} else {
		ofDrawBitmapString("searching for face...", 10, 20);
	}
}

void testApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}