#include "ofApp.h"

using namespace ofxCv;

void ofApp::setup() {
    dir.allowExt("png");
    dir.allowExt("jpg");
    dir.allowExt("tiff");
    dir.listDir("faces/");
    index = 0;
	tracker.setup();
    tracker.setRescale(.5);
    tracker.setIterations(1);
    tracker.setClamp(4);
    tracker.setTolerance(.01);
    tracker.setAttempts(4);
    iterations = 30;
}

void ofApp::saveFace() {
    ofMesh mesh = tracker.getImageMesh();
    string facePly = dir.getFile(index).path() + ".ply";
    mesh.save(facePly);
}

void ofApp::loadFace() {
    string facePly = dir.getFile(index).path() + ".ply";
    if(ofFile(facePly).exists()) {
        prevFace.load(facePly);
    }
}

void ofApp::update() {
    if(index < dir.size()) {
        tracker.reset();
        img.load(dir.getPath(index));
        for(int i = 0; i < iterations; i++) {
            tracker.update(toCv(img));
        }
        loadFace();
        saveFace();
        timer.tick();
        index++;
    }
}

void ofApp::draw() {
    ofSetColor(255);
    if(img.isAllocated()) {
        img.draw(0, 0);
    }
    ofSetColor(ofColor::white);
    tracker.getImageMesh().drawVertices();
    ofSetColor(ofColor::white, 16);
    tracker.getImageMesh().drawWireframe();
    
    ofSetColor(ofColor::yellow);
    prevFace.drawVertices();
    ofSetColor(ofColor::yellow, 16);
    prevFace.drawWireframe();

    int n = dir.size();
    float timeEstimate = timer.getPeriod() * (n - index);
    ofDrawBitmapStringHighlight(ofToString(index) + "/" + ofToString(n) + " " + ofToString((int) timeEstimate) + "s", 10, 20);
}
