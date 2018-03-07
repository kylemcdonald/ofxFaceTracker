#include "ofApp.h"
#include "ofxXmlPoco.h"

using namespace ofxCv;
using namespace cv;

void ofApp::loadSettings() {
    // if you want to package the app by itself without an outer
    // folder, you can place the "data" folder inside the app in
    // the Resources folder (right click on the app, "show contents")
    //ofSetDataPathRoot("../Resources/data/");
    
    // setup gui
    bGuiVisible = true;
    gui.setup();
    gui.setName("FaceOSC");
    gui.setPosition(0, 0);
    gui.add(bIncludePose.set("pose", true));
    gui.add(bIncludeGestures.set("gesture", true));
    gui.add(bIncludeAllVertices.set("raw", false));
    gui.add(bNormalizeRaw.set("normalize raw", false));
    
    // load settings file
	ofxXmlPoco xml;
	if(!xml.load(ofToDataPath("settings.xml"))) {
		return;
	}
	
	// expects following tags to be wrapped by a main "faceosc" tag
	
	bool bUseCamera = true;

	xml.setTo("source");
	if(xml.exists("useCamera")) {
		bUseCamera = xml.getValue("useCamera", false);
	}
	xml.setToParent();

	xml.setTo("camera");
	if(xml.exists("device")) {
		cam.setDeviceID(xml.getValue("device", 0));
	}
	if(xml.exists("framerate")) {
		cam.setDesiredFrameRate(xml.getValue("framerate", 30));
	}
	camWidth = xml.getValue("width", 640);
	camHeight = xml.getValue("height", 480);
	cam.initGrabber(camWidth, camHeight);
	xml.setToParent();

	xml.setTo("movie");
	if(xml.exists("filename")) {
		string filename = ofToDataPath((string)xml.getValue("filename", ""));
		if(filename != "") {
			if(!movie.load(filename)) {
				ofLog(OF_LOG_ERROR, "Could not load movie \"%s\", reverting to camera input", filename.c_str());
				bUseCamera = true;
			}
			movie.play();
		}
	}
	else {
		ofLog(OF_LOG_ERROR, "Movie filename tag not set in settings, reverting to camera input");
		bUseCamera = true;
	}
	if(xml.exists("volume")) {
		float movieVolume = ofClamp(xml.getValue("volume", 1.0), 0, 1.0);
		movie.setVolume(movieVolume);
	}
	if(xml.exists("speed")) {
		float movieSpeed = ofClamp(xml.getValue("speed", 1.0), -16, 16);
		movie.setSpeed(movieSpeed);
	}
	bPaused = false;
	movieWidth = movie.getWidth();
	movieHeight = movie.getHeight();
	xml.setToParent();

	if(bUseCamera) {
		ofSetWindowShape(camWidth, camHeight);
		setVideoSource(true);
	}
	else {
		ofSetWindowShape(movieWidth, movieHeight);
		setVideoSource(false);
	}

	xml.setTo("face");
	if(xml.exists("rescale")) {
		tracker.setRescale(xml.getValue("rescale", 1.0));
	}
	if(xml.exists("iterations")) {
		tracker.setIterations(xml.getValue("iterations", 5));
	}
	if(xml.exists("clamp")) {
		tracker.setClamp(xml.getValue("clamp", 3.0));
	}
	if(xml.exists("tolerance")) {
		tracker.setTolerance(xml.getValue("tolerance", 0.01));
	}
	if(xml.exists("attempts")) {
		tracker.setAttempts(xml.getValue("attempts", 1));
	}
	bDrawMesh = true;
	if(xml.exists("drawMesh")) {
		bDrawMesh = (bool) xml.getValue("drawMesh", 1);
	}
	tracker.setup();
	xml.setToParent();

	xml.setTo("osc");
	host = xml.getValue("host", "localhost");
	port = xml.getValue("port", 8338);
	osc.setup(host, port);
	xml.setToParent();
	
	xml.clear();
}

void ofApp::setup() {
	ofSetVerticalSync(true);
	loadSettings();
}

void ofApp::update() {
	if(bPaused)
		return;

	videoSource->update();
	if(videoSource->isFrameNew()) {
		tracker.update(toCv(*videoSource));
        sendFaceOsc(tracker);
		rotationMatrix = tracker.getRotationMatrix();
	}
}

void ofApp::draw() {
	ofSetColor(255);
	videoSource->draw(0, 0);

	if(tracker.getFound()) {
        ofDrawBitmapString("framerate "+ofToString((int) ofGetFrameRate())+", sending OSC to "+host+":"+ofToString(port), 10, ofGetHeight()-4);

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
        ofSetColor(255);
        ofDrawBitmapString("searching for face...", 10, ofGetHeight()-4);
	}

	if(bPaused) {
		ofSetColor(255, 0, 0);
		ofDrawBitmapString( "paused", 10, 32);
	}

	if(!bUseCamera) {
		ofSetColor(255, 0, 0);
		ofDrawBitmapString("speed "+ofToString(movie.getSpeed()), ofGetWidth()-100, 20);
	}
    
    if(bGuiVisible) {
        gui.draw();
    }
}

void ofApp::keyPressed(int key) {
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
        case 'g':
            bGuiVisible = !bGuiVisible;
            break;
		case OF_KEY_UP:
			movie.setSpeed(ofClamp(movie.getSpeed()+0.2, -16, 16));
			break;
		case OF_KEY_DOWN:
			movie.setSpeed(ofClamp(movie.getSpeed()-0.2, -16, 16));
			break;
	}
}

void ofApp::setVideoSource(bool useCamera) {

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
