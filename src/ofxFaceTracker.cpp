#include "ofxFaceTracker.h"

// can be compiled with OpenMP for even faster threaded execution

#define it at<int>
#define db at<double>

ofxFaceTracker::ofxFaceTracker()
:scale(1)
,iterations(5) // [1-25] 1 is fast and inaccurate, 25 is slow and accurate
,clamp(3) // [0-4] 1 gives a very loose fit, 4 gives a very tight fit
,tolerance(.01) // [.01-1] match tolerance
,attempts(1) // [1-4] 1 is fast and may not find faces, 4 is slow but will find faces
,failed(true)
,fcheck(true) // check for whether the tracking failed
,frameSkip(-1) // how often to skip frames
{}

void ofxFaceTracker::setup() {
	wSize1.resize(1);
	wSize1[0] = 7;
	
	wSize2.resize(3);
	wSize2[0] = 11;
	wSize2[1] = 9;
	wSize2[2] = 7;
	
	string ftFile = ofToDataPath("model/face2.tracker");
	string triFile = ofToDataPath("model/face.tri");
	string conFile = ofToDataPath("model/face.con");
	
	tracker.Load(ftFile.c_str());
	tri = IO::LoadTri(triFile.c_str());
	con = IO::LoadCon(conFile.c_str());  // not being used right now
}

void ofxFaceTracker::update(Mat image) {	
	if(scale == 1) {
		im = image; 
	} else {
		resize(image, im, cv::Size(scale * image.cols, scale * image.rows));
	}
	
	cvtColor(im, gray, CV_RGB2GRAY);
	
	bool tryAgain = true;
	for(int i = 0; tryAgain && i < attempts; i++) {
		vector<int> wSize;
		if(failed) {
			wSize = wSize2;
		} else {
			wSize = wSize1;
		}
		
		if(tracker.Track(gray, wSize, frameSkip, iterations, clamp, tolerance, fcheck) == 0) {
			idx = tracker._clm.GetViewIdx();
			failed = false;
			tryAgain = false;
			updateObjectPoints();
		} else {
			tracker.FrameReset();
			failed = true;
		}
	}
}

void ofxFaceTracker::draw() const {
	if(failed) {
		return;
	}
	getImageMesh().drawWireframe();
	int n = size();
	for(int i = 0; i < n; i++){
		if(getVisibility(i)) {
			ofDrawBitmapString(ofToString(i), getImagePoint(i));
		}
	}
}

void ofxFaceTracker::reset() {
	tracker.FrameReset();
}

int ofxFaceTracker::size() const {
	return tracker._shape.rows / 2;
}

bool ofxFaceTracker::getFound() const {
	return !failed;
}

bool ofxFaceTracker::getVisibility(int i) const {
	if(failed) {
		return false;
	}
	const Mat& visi = tracker._clm._visi[idx];
	return (visi.it(i, 0) != 0);
}

ofVec2f ofxFaceTracker::getImagePoint(int i) const {
	if(failed) {
		return ofVec2f();
	}
	const Mat& shape = tracker._shape;
	int n = size();
	return ofVec2f(shape.db(i, 0), shape.db(i + n, 0)) / scale;
}

ofVec3f ofxFaceTracker::getObjectPoint(int i) const {
	if(failed) {
		return ofVec3f();
	}	
	int n = objectPoints.rows / 3;
	return ofVec3f(objectPoints.db(i,0), objectPoints.db(i+n,0), objectPoints.db(i+n+n,0));
}

ofMesh ofxFaceTracker::getImageMesh() const {
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	
	if(!failed) {
		for(int i = 0; i < tri.rows; i++){
			if(getVisibility(tri.it(i,0)) &&
				 getVisibility(tri.it(i,1)) &&
				 getVisibility(tri.it(i,2))) {
				mesh.addVertex(getImagePoint(tri.it(i,0)));
				mesh.addVertex(getImagePoint(tri.it(i,1)));
				mesh.addVertex(getImagePoint(tri.it(i,2)));
			}
		}
	}
	
	return mesh;
}

ofMesh ofxFaceTracker::getObjectMesh() const {
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	
	if(!failed) {
		for(int i = 0; i < tri.rows; i++){
			if(getVisibility(tri.it(i,0)) &&
				 getVisibility(tri.it(i,1)) &&
				 getVisibility(tri.it(i,2))) {
				mesh.addVertex(getObjectPoint(tri.it(i,0)));
				mesh.addVertex(getObjectPoint(tri.it(i,1)));
				mesh.addVertex(getObjectPoint(tri.it(i,2)));
			}
		}
	}
	
	return mesh;
}

ofVec2f ofxFaceTracker::getPosition() const {
	const Mat& pose = tracker._clm._pglobl;
	return ofVec2f(pose.db(4,0), pose.db(5,0));
}

float ofxFaceTracker::getScale() const {
	const Mat& pose = tracker._clm._pglobl;
	return pose.db(0,0);
}

ofVec3f ofxFaceTracker::getOrientation() const {
	const Mat& pose = tracker._clm._pglobl;
	ofVec3f euler(pose.db(1, 0), pose.db(2, 0), pose.db(3, 0));
	return euler;
}

ofMatrix4x4 ofxFaceTracker::getRotationMatrix() const {
	ofVec3f euler = getOrientation();
	ofMatrix4x4 matrix;
	matrix.makeRotationMatrix(ofRadToDeg(euler.x), ofVec3f(1,0,0),
														ofRadToDeg(euler.y), ofVec3f(0,1,0),
														ofRadToDeg(euler.z), ofVec3f(0,0,1));
														return matrix;
}

ofPolyline ofxFaceTracker::getFeature(Feature feature) const {
	if(failed) {
		return ofPolyline();
	}
	int begin, end;
	switch(feature) {
		case LEFT_JAW: begin = 0; end = 9; break;
		case RIGHT_JAW: begin = 8; end = 17; break;
		case JAW: begin = 0; end = 17; break;
		case LEFT_EYEBROW: begin = 17; end = 22; break;
		case RIGHT_EYEBROW: begin = 22; end = 27; break;
		case LEFT_EYE: begin = 36; end = 42; break;
		case RIGHT_EYE: begin = 42; end = 48; break;
		case OUTER_MOUTH: begin = 48; end = 60; break;
	}
	ofPolyline result;
	for(int i = begin; i < end; i++) {
		if(getVisibility(i)) {
			result.addVertex(getImagePoint(i));
		}
	}
	return result;
}

float ofxFaceTracker::getGesture(Gesture gesture) const {
	if(failed) {
		return 0;
	}
	int start = 0, end = 0;
	switch(gesture) {
		// left to right of mouth
		case MOUTH_WIDTH:
		start = (48);
		end = (54);
		break;
				
		 // top to bottom of inner mouth
		case MOUTH_HEIGHT:
		start = (61);
		end = (64);
		break;
		
		// center of the eye to middle of eyebrow
		case LEFT_EYEBROW_HEIGHT:
		start = (38);
		end = (20);
		break;
		
		// center of the eye to middle of eyebrow
		case RIGHT_EYEBROW_HEIGHT:
		start = (43);
		end = (23);
		break;
		
		// upper inner eye to lower outer eye
		case LEFT_EYE_OPENNESS:
		start = (38);
		end = (41);
		break;
		
		// upper inner eye to lower outer eye
		case RIGHT_EYE_OPENNESS:
		start = (43);
		end = (46);
		break;
		
		// nose center to chin center
		case JAW_OPENNESS:
		start = (33);
		end = (8);
		break;
		
		// left side of nose to right side of nose
		case NOSTRIL_FLARE:
		start = (31);
		end = (35);
		break;
	}
	
	return (getObjectPoint(start) - getObjectPoint(end)).length();
}

void ofxFaceTracker::setScale(float scale) {
	this->scale = scale;
}

void ofxFaceTracker::setIterations(int iterations) {
	this->iterations = iterations;
}

void ofxFaceTracker::setClamp(float clamp) {
	this->clamp = clamp;
}

void ofxFaceTracker::setTolerance(float tolerance) {
	this->tolerance = tolerance;
}

void ofxFaceTracker::setAttempts(int attempts) {
	this->attempts = attempts;
}

void ofxFaceTracker::updateObjectPoints() {
	const Mat& mean = tracker._clm._pdm._M;
	const Mat& variation = tracker._clm._pdm._V;
	const Mat& weights = tracker._clm._plocal;
	objectPoints = mean + variation * weights;
}