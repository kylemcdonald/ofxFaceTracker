#include "ofxTracker.h"

// can be compiled with OpenMP for even faster threaded execution

#define it at<int>
#define db at<double>

ofxTracker::ofxTracker()
:scale(1)
,iterations(5) // [1-25] 1 is fast and inaccurate, 25 is slow and accurate
,clamp(3) // [0-4] 1 gives a very loose fit, 4 gives a very tight fit
,tolerance(.01) // [.01-1] match tolerance
,attempts(1) // [1-4] 1 is fast and may not find faces, 4 is slow but will find faces
,failed(true)
,fcheck(true) // check for whether the tracking failed
,frameSkip(-1) // how often to skip frames
{}

void ofxTracker::setup() {
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

void ofxTracker::update(Mat image) {	
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

void ofxTracker::draw() const {
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

void ofxTracker::reset() {
	tracker.FrameReset();
}

int ofxTracker::size() const {
	return tracker._shape.rows / 2;
}

bool ofxTracker::getFound() const {
	return !failed;
}

bool ofxTracker::getVisibility(int i) const {
	if(failed) {
		return false;
	}
	const Mat& visi = tracker._clm._visi[idx];
	return (visi.it(i, 0) != 0);
}

ofVec2f ofxTracker::getImagePoint(int i) const {
	if(failed) {
		return ofVec2f();
	}
	const Mat& shape = tracker._shape;
	int n = size();
	return ofVec2f(shape.db(i, 0), shape.db(i + n, 0)) / scale;
}

ofVec3f ofxTracker::getObjectPoint(int i) const {
	if(failed) {
		return ofVec3f();
	}	
	int n = objectPoints.rows / 3;
	return ofVec3f(objectPoints.db(i,0), objectPoints.db(i+n,0), objectPoints.db(i+n+n,0));
}

ofMesh ofxTracker::getImageMesh() const {
	ofMesh mesh;
	mesh.setMode(OF_TRIANGLES_MODE);
	
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

ofMesh ofxTracker::getObjectMesh() const {
	ofMesh mesh;
	mesh.setMode(OF_TRIANGLES_MODE);
	
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

ofVec2f ofxTracker::getPosition() const {
	const Mat& pose = tracker._clm._pglobl;
	return ofVec2f(pose.db(4,0), pose.db(5,0));
}

float ofxTracker::getScale() const {
	const Mat& pose = tracker._clm._pglobl;
	return pose.db(0,0);
}

void ofxTracker::loadCalibration(string filename) {
	calibration.load(filename);
}

void ofxTracker::getCalibratedPose(Mat& rotation, Mat& translation) const {
	if(failed) {
		return;
	}
	
	vector<Point3f> objectPoints;
	vector<Point2f> imagePoints;
	
	const int indicesInit[] = {
		36, 39, // left eye
		42, 45, // right eye
		30}; // nose
	const int indicesSize = 5; // >= 4 for solvePnP
	const vector<int> indices(indicesInit, &indicesInit[indicesSize]);
	
	int n = size();
	for(int i = 0; i < indices.size(); i++) {
		int j = indices[i];
		objectPoints.push_back(toCv(getObjectPoint(j)));
		imagePoints.push_back(toCv(getImagePoint(j)));
	}
	
	const Mat& cameraMatrix = calibration.getDistortedIntrinsics().getCameraMatrix();
	const Mat& distCoeffs = calibration.getDistCoeffs();
	// need to use rotiation from getOrientation as an initial guess
	
	Mat guess = Mat(3, 3, CV_64FC1);
	ofMatrix4x4 orientation;
	getOrientation(orientation);
	for(int i = 0; i < 3; i++) {
		for(int j = 0; j < 3; j++) {
			guess.db(i, j) = orientation(i, j);
		}
	}
	Rodrigues(guess, rotation);
	
	solvePnP(Mat(objectPoints), Mat(imagePoints), cameraMatrix, distCoeffs, rotation, translation);
}

void ofxTracker::getOrientation(ofVec3f& eulerAngles) const {
	const Mat& pose = tracker._clm._pglobl;
	eulerAngles.set(pose.db(1, 0), pose.db(2, 0), pose.db(3, 0));
}

void ofxTracker::getOrientation(ofMatrix4x4& rotationMatrix) const {
	ofVec3f euler;
	getOrientation(euler);
	rotationMatrix.makeRotationMatrix(ofRadToDeg(euler.x), ofVec3f(1,0,0),
																		ofRadToDeg(euler.y), ofVec3f(0,1,0),
																		ofRadToDeg(euler.z), ofVec3f(0,0,1));
}

ofPolyline ofxTracker::getFeature(Feature feature) const {
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

void ofxTracker::setScale(float scale) {
	this->scale = scale;
}

void ofxTracker::setIterations(int iterations) {
	this->iterations = iterations;
}

void ofxTracker::setClamp(float clamp) {
	this->clamp = clamp;
}

void ofxTracker::setTolerance(float tolerance) {
	this->tolerance = tolerance;
}

void ofxTracker::setAttempts(int attempts) {
	this->attempts = attempts;
}

void ofxTracker::updateObjectPoints() {
	const Mat& mean = tracker._clm._pdm._M;
	const Mat& variation = tracker._clm._pdm._V;
	const Mat& weights = tracker._clm._plocal;
	objectPoints = mean + variation * weights;
}