#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

void ofApp::loadSettings() {
    ofSetDataPathRoot("../Resources/data/");
    
    bGuiVisible = true;
    gui.setup();
    gui.setName("FaceOSC");
    gui.setPosition(0, 0);
    gui.add(bIncludePose.set("pose", true));
    gui.add(bIncludeGestures.set("gesture", true));
    gui.add(bIncludeAllVertices.set("raw", false));
#ifdef TARGET_OSX
    gui.add(bSyphon.set("syphon", false));
#endif

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
        if(!movie.load(filename)) {
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
    if(xml.getNumTags("bIncludePose")) {
        bIncludePose = (bool) xml.getValue("bIncludePose", bIncludePose);
    }
    if(xml.getNumTags("bIncludeGestures")) {
        bIncludeGestures = (bool) xml.getValue("bIncludeGestures", bIncludeGestures);
    }
    if(xml.getNumTags("bIncludeAllVertices")) {
        bIncludeAllVertices = (bool) xml.getValue("bIncludeAllVertices", bIncludeAllVertices);
    }
    if(xml.getNumTags("bSyphon")) {
        bSyphon = (bool) xml.getValue("bSyphon", bSyphon);
    }
    tracker.setup();
    xml.popTag();
    
    xml.pushTag("osc");
    host = xml.getValue("host", "localhost");
    port = xml.getValue("port", 8338);
    osc.setup(host, port);
    xml.popTag();
    
    osc.setup(host, port);
    
#ifdef TARGET_OSX
    syphonServer.setName("FaceOSC Camera");
#endif
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
        ofDrawBitmapString(ofToString((int) ofGetFrameRate()), ofGetWidth()-20, 20);
        
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
        ofDrawBitmapString("searching for face...", 240, 20);
    }
    
    if(bPaused) {
        ofSetColor(255, 0, 0);
        ofDrawBitmapString( "paused", 240, 32);
    }
    
    if(!bUseCamera) {
        ofSetColor(255, 0, 0);
        ofDrawBitmapString("speed "+ofToString(movie.getSpeed()), ofGetWidth()-100, 20);
    }
    
    if(bGuiVisible) {
        gui.draw();
    }
    
#ifdef TARGET_OSX
    if(bSyphon) {
        syphonServer.publishScreen();
    }
#endif
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