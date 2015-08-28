#include "testApp.h"

using namespace ofxCv;
using namespace cv;

void testApp::loadSettings() {
	ofxXmlSettings xml;
	xml.loadFile("settings.xml");

	bool bUseCamera = true;

	xml.pushTag("source");
	if(xml.getNumTags("useCamera") > 0) {
		bUseCamera = xml.getValue("useCamera", 0);
	}
	xml.popTag();

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
	xml.popTag();

	xml.pushTag("movie");
	if(xml.getNumTags("filename") > 0) {
		string filename = ofToDataPath((string) xml.getValue("filename", ""));
		if(!movie.loadMovie(filename)) {
			ofLog(OF_LOG_ERROR, "Could not load movie \"%s\", reverting to camera input", filename.c_str());
			bUseCamera = true;
		}
		movie.play();
	}
	else {
		ofLog(OF_LOG_ERROR, "Movie filename tag not set in settings, reverting to camera input");
		bUseCamera = true;
	}
	if(xml.getNumTags("volume") > 0) {
		float movieVolume = ofClamp(xml.getValue("volume", 1.0), 0, 1.0);
		movie.setVolume(movieVolume);
	}
	if(xml.getNumTags("speed") > 0) {
		float movieSpeed = ofClamp(xml.getValue("speed", 1.0), -16, 16);
		movie.setSpeed(movieSpeed);
	}
	bPaused = false;
	movieWidth = movie.getWidth();
	movieHeight = movie.getHeight();
	xml.popTag();

	if(bUseCamera) {
		ofSetWindowShape(camWidth, camHeight);
		setVideoSource(true);
	}
	else {
		ofSetWindowShape(movieWidth, movieHeight);
		setVideoSource(false);
	}

	xml.pushTag("face");
	if(xml.getNumTags("rescale")) {
		tracker.setRescale(xml.getValue("rescale", 1.));
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
	bDrawMesh = true;
	if(xml.getNumTags("drawMesh")) {
		bDrawMesh = (bool) xml.getValue("drawMesh", 1);
	}
	tracker.setup();
	xml.popTag();

	xml.pushTag("osc");
	host = xml.getValue("host", "localhost");
	port = xml.getValue("port", 8338);
	osc.setup(host, port);
	xml.popTag();

	osc.setup(host, port);
}

void testApp::setup() {
	ofSetVerticalSync(true);
	loadSettings();
}

void testApp::update() {
	if(bPaused)
		return;

	videoSource->update();
	if(videoSource->isFrameNew()) {
		tracker.update(toCv(*videoSource));
        sendFaceOsc(tracker);
		rotationMatrix = tracker.getRotationMatrix();
	}
}

void testApp::draw() {
	ofSetColor(255);
	videoSource->draw(0, 0);

	if(tracker.getFound()) {
		ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);

		if(bDrawMesh) {
			ofSetLineWidth(1);
			//tracker.draw();
			tracker.getImageMesh().drawWireframe();

			ofPushView();
			ofSetupScreenOrtho(sourceWidth, sourceHeight, -1000, 1000);
			ofVec2f pos = tracker.getPosition();
			ofTranslate(pos.x, pos.y);
			applyMatrix(rotationMatrix);
			ofScale(10,10,10);
			ofDrawAxis(tracker.getScale());
			ofPopView();
		}
	} else {
		ofDrawBitmapString("searching for face...", 10, 20);
	}

	if(bPaused) {
		ofSetColor(255, 0, 0);
		ofDrawBitmapString( "paused", 10, 32);
	}

	if(!bUseCamera) {
		ofSetColor(255, 0, 0);
		ofDrawBitmapString("speed "+ofToString(movie.getSpeed()), ofGetWidth()-100, 20);
	}
}

void testApp::keyPressed(int key) {
	switch(key) {
		case 'r':
			tracker.reset();
			break;
		case 'm':
			bDrawMesh = !bDrawMesh;
			break;
		case 'p':
			bPaused = !bPaused;
			break;
		case OF_KEY_UP:
			movie.setSpeed(ofClamp(movie.getSpeed()+0.2, -16, 16));
			break;
		case OF_KEY_DOWN:
			movie.setSpeed(ofClamp(movie.getSpeed()-0.2, -16, 16));
			break;
	}
}

void testApp::setVideoSource(bool useCamera) {

	bUseCamera = useCamera;

	if(bUseCamera) {
		videoSource = &cam;
		sourceWidth = camWidth;
		sourceHeight = camHeight;
	}
	else {
		videoSource = &movie;
		sourceWidth = movieWidth;
		sourceHeight = movieHeight;
	}
}
