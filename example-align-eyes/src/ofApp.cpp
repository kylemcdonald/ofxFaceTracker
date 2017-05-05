#include "ofApp.h"

using namespace ofxCv;

void ofApp::setup() {
	tracker.setup();
    std::size_t n = dir.listDir("faces/");

    if (n == 0)
    {
        ofLogError() << "No files in the directory. Quitting.";
        ofExit();
    }

    cur = 0;
    eyeDistance = 32;
    fbo.allocate(ofGetWidth(), ofGetHeight());
}

void ofApp::update() {
    string filename = dir.getFile(cur).getFileName();
    img.load("faces/" + filename);
    tracker.reset();
    if(tracker.update(toCv(img))) {
        fbo.begin();
        ofClear(255, 255);
        ofPushMatrix();
        ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
        ofVec2f leftEye = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE).getCentroid2D();
        ofVec2f rightEye = tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE).getCentroid2D();
        float curDistance = leftEye.distance(rightEye);
        float scale = eyeDistance / curDistance;
        float rotation = (rightEye - leftEye).angle(ofVec2f(1, 0));
        ofTranslate(-eyeDistance / 2, 0);
        ofScale(scale, scale);
        ofRotateDeg(rotation);
        ofTranslate(-leftEye.x, -leftEye.y);
        img.draw(0, 0);
        ofPopMatrix();
        fbo.end();
        fbo.readToPixels(pix);
        pix.setImageType(OF_IMAGE_GRAYSCALE);
        ofSaveImage(pix, "aligned/" + ofToString(tracker.getOrientation().y) + "_" + filename + ".tiff");
    }
    cur++;
    if(cur == dir.size()) {
        cur = 0;
        ofLog() << "done";
        ofExit();
    }
}

void ofApp::draw() {
    fbo.draw(0, 0);
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
}

void ofApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}
