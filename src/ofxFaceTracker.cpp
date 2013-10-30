#include "ofxFaceTracker.h"

using namespace ofxCv;
using namespace cv;
using namespace FACETRACKER;

// can be compiled with OpenMP for even faster threaded execution

#define it at<int>
#define db at<double>

vector<int> consecutive(int start, int end) {
	int n = end - start;
	vector<int> result(n);
	for(int i = 0; i < n; i++) {
		result[i] = start + i;
	}
	return result;
}

vector<int> ofxFaceTracker::getFeatureIndices(Feature feature) {
	switch(feature) {
		case LEFT_JAW: return consecutive(0, 9);
		case RIGHT_JAW: return consecutive(8, 17);
		case JAW: return consecutive(0, 17);
		case LEFT_EYEBROW: return consecutive(17, 22);
		case RIGHT_EYEBROW: return consecutive(22, 27);
		case LEFT_EYE: return consecutive(36, 42);
		case RIGHT_EYE: return consecutive(42, 48);
		case OUTER_MOUTH: return consecutive(48, 60);
		case INNER_MOUTH: {
			static int innerMouth[] = {48,60,61,62,54,63,64,65};
			return vector<int>(innerMouth, innerMouth + 8);
		}
		case NOSE_BRIDGE: return consecutive(27, 31);
		case NOSE_BASE: return consecutive(31, 36);
		case FACE_OUTLINE: {
			static int faceOutline[] = {17,18,19,20,21,22,23,24,25,26, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
			return vector<int>(faceOutline, faceOutline + 27);
		}
		case ALL_FEATURES: return consecutive(0, 66);
	}
}
		
ofxFaceTracker::ofxFaceTracker()
:rescale(1)
,iterations(10) // [1-25] 1 is fast and inaccurate, 25 is slow and accurate
,clamp(3) // [0-4] 1 gives a very loose fit, 4 gives a very tight fit
,tolerance(.01) // [.01-1] match tolerance
,attempts(1) // [1-4] 1 is fast and may not find faces, 4 is slow but will find faces
,failed(true)
,fcheck(true) // check for whether the tracking failed
,frameSkip(-1) // how often to skip frames
,useInvisible(true)
,age(-1)
{
}

void ofxFaceTracker::setup() {
	wSize1.resize(1);
	wSize1[0] = 7;
	
	wSize2.resize(3);
	wSize2[0] = 11;
	wSize2[1] = 9;
	wSize2[2] = 7;
	
	if(!ofFile("model/face2.tracker").exists() ||
		 !ofFile("model/face.tri").exists() ||
		 !ofFile("model/face.con").exists()) {
		ofLogError() << "Make sure you've placed the files face2.tracker, face.tri and face.con in the data/model/ folder.";
	}
	
	string ftFile = ofToDataPath("model/face2.tracker");
	string triFile = ofToDataPath("model/face.tri");
	string conFile = ofToDataPath("model/face.con");
	
	tracker.Load(ftFile.c_str());
	tri = IO::LoadTri(triFile.c_str());
	con = IO::LoadCon(conFile.c_str());  // not being used right now
}

bool ofxFaceTracker::update(Mat image) {	
	if(rescale == 1) {
		im = image; 
	} else {
		resize(image, im, cv::Size(rescale * image.cols, rescale * image.rows));
	}
	
	if(im.type() == CV_8UC3) {
		cvtColor(im, gray, CV_RGB2GRAY);
	} else if(im.type() == CV_8UC1) {
		gray = im;
	}
		
	bool tryAgain = true;
	for(int i = 0; tryAgain && i < attempts; i++) {
		vector<int> wSize;
		if(failed) {
			wSize = wSize2;
		} else {
			wSize = wSize1;
		}
		
		if(tracker.Track(gray, wSize, frameSkip, iterations, clamp, tolerance, fcheck) == 0) {
			currentView = tracker._clm.GetViewIdx();
			failed = false;
			age++;
			tryAgain = false;
			updateObjectPoints();
		} else {
			tracker.FrameReset();
			failed = true;
			age = -1;
		}
	}
	return !failed;
}

void ofxFaceTracker::draw(bool drawLabels) const{
	if(failed) {
		return;
	}
	
	ofPushStyle();
	ofNoFill();
	getImageFeature(LEFT_EYE).draw();
	getImageFeature(RIGHT_EYE).draw();
	getImageFeature(LEFT_EYEBROW).draw();
	getImageFeature(RIGHT_EYEBROW).draw();
	getImageFeature(NOSE_BRIDGE).draw();
	getImageFeature(NOSE_BASE).draw();
	getImageFeature(INNER_MOUTH).draw();
	getImageFeature(OUTER_MOUTH).draw();
	getImageFeature(JAW).draw();
	ofPopStyle();
	
	if(drawLabels) {
		int n = size();
		for(int i = 0; i < n; i++){
			if(getVisibility(i)) {
				ofDrawBitmapString(ofToString(i), getImagePoint(i));
			}
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

bool ofxFaceTracker::getHaarFound() const {
	return tracker._fdet._haar_count > 0;
}

int ofxFaceTracker::getAge() const {
	return age;
}

bool ofxFaceTracker::getVisibility(int i) const {
	if(failed) {
		return false;
	}
	const Mat& visi = tracker._clm._visi[currentView];
	return (visi.it(i, 0) != 0);
}

vector<ofVec2f> ofxFaceTracker::getImagePoints() const {
	int n = size();
	vector<ofVec2f> imagePoints(n);
	for(int i = 0; i < n; i++) {
		imagePoints[i] = getImagePoint(i);
	}
	return imagePoints;
}

vector<ofVec3f> ofxFaceTracker::getObjectPoints() const {
	int n = size();
	vector<ofVec3f> objectPoints(n);
	for(int i = 0; i < n; i++) {
		objectPoints[i] = getObjectPoint(i);
	}
	return objectPoints;
}

vector<ofVec3f> ofxFaceTracker::getMeanObjectPoints() const {
	int n = size();
	vector<ofVec3f> meanObjectPoints(n);
	for(int i = 0; i < n; i++) {
		meanObjectPoints[i] = getMeanObjectPoint(i);
	}
	return meanObjectPoints;
}

ofVec2f ofxFaceTracker::getImagePoint(int i) const {
	if(failed) {
		return ofVec2f();
	}
	const Mat& shape = tracker._shape;
	int n = shape.rows / 2;
	return ofVec2f(shape.db(i, 0), shape.db(i + n, 0)) / rescale;
}

ofVec3f ofxFaceTracker::getObjectPoint(int i) const {
	if(failed) {
		return ofVec3f();
	}	
	int n = objectPoints.rows / 3;
	return ofVec3f(objectPoints.db(i,0), objectPoints.db(i+n,0), objectPoints.db(i+n+n,0));
}

ofVec3f ofxFaceTracker::getMeanObjectPoint(int i) const {
	const Mat& mean = tracker._clm._pdm._M;
	int n = mean.rows / 3;
	return ofVec3f(mean.db(i,0), mean.db(i+n,0), mean.db(i+n+n,0));
}

ofMesh ofxFaceTracker::getImageMesh() const{
	return getMesh(getImagePoints());
}

ofMesh ofxFaceTracker::getObjectMesh() const {
	return getMesh(getObjectPoints());
}

ofMesh ofxFaceTracker::getMeanObjectMesh() const {
	return getMesh(getMeanObjectPoints());
}

const Mat& ofxFaceTracker::getObjectPointsMat() const {
	return objectPoints;
}

ofRectangle ofxFaceTracker::getHaarRectangle() const {
	cv::Rect rect = tracker._fdet._haar_rect;
	return ofRectangle(rect.x / rescale, rect.y / rescale, rect.width / rescale, rect.height / rescale);
}

ofVec2f ofxFaceTracker::getPosition() const {
	const Mat& pose = tracker._clm._pglobl;
	return ofVec2f(pose.db(4,0), pose.db(5,0)) / rescale;
}

 // multiply by ~20-23 to get pixel units (+/-20 units in the x axis, +23/-18 on the y axis)
float ofxFaceTracker::getScale() const {
	const Mat& pose = tracker._clm._pglobl;
	return pose.db(0,0) / rescale;
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

ofxFaceTracker::Direction ofxFaceTracker::getDirection() const {
	if(failed) {
		return FACING_UNKNOWN;
	}
	switch(currentView) {
		case 0: return FACING_FORWARD;
		case 1: return FACING_LEFT;
		case 2: return FACING_RIGHT;
	}
}

ofPolyline ofxFaceTracker::getImageFeature(Feature feature) const {
	return getFeature(feature, getImagePoints());
}

ofPolyline ofxFaceTracker::getObjectFeature(Feature feature) const {
	return getFeature(feature, getObjectPoints());
}

ofPolyline ofxFaceTracker::getMeanObjectFeature(Feature feature) const {
	return getFeature(feature, getMeanObjectPoints());
}

float ofxFaceTracker::getGesture(Gesture gesture) const {
	if(failed) {
		return 0;
	}
	int start = 0, end = 0;
	switch(gesture) {
		// left to right of mouth
		case MOUTH_WIDTH: start = 48; end = 54; break;
		 // top to bottom of inner mouth
		case MOUTH_HEIGHT: start = 61; end = 64; break;
		// center of the eye to middle of eyebrow
		case LEFT_EYEBROW_HEIGHT: start = 38; end = 20; break;
		// center of the eye to middle of eyebrow
		case RIGHT_EYEBROW_HEIGHT: start = 43; end = 23; break;
		// upper inner eye to lower outer eye
		case LEFT_EYE_OPENNESS: start = 38; end = 41; break;
		// upper inner eye to lower outer eye
		case RIGHT_EYE_OPENNESS: start = 43; end = 46; break;
		// nose center to chin center
		case JAW_OPENNESS: start = 33; end = 8; break;
		// left side of nose to right side of nose
		case NOSTRIL_FLARE: start = 31; end = 35; break;
	}
	
	return (getObjectPoint(start) - getObjectPoint(end)).length();
}

void ofxFaceTracker::setRescale(float rescale) {
	this->rescale = rescale;
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

void ofxFaceTracker::setUseInvisible(bool useInvisible) {
	this->useInvisible = useInvisible;
}

void ofxFaceTracker::setHaarMinSize(float minSize) {
	tracker._fdet._min_size = minSize * rescale;
}

void ofxFaceTracker::updateObjectPoints() {
	const Mat& mean = tracker._clm._pdm._M;
	const Mat& variation = tracker._clm._pdm._V;
	const Mat& weights = tracker._clm._plocal;
	objectPoints = mean + variation * weights;
}

void ofxFaceTracker::addTriangleIndices(ofMesh& mesh) const {
	int in = tri.rows;
	for(int i = 0; i < tri.rows; i++) {
		int i0 = tri.it(i, 0), i1 = tri.it(i, 1), i2 = tri.it(i, 2);
		bool visible = getVisibility(i0) && getVisibility(i1) && getVisibility(i2);
		if(useInvisible || visible) {
			mesh.addIndex(i0);
			mesh.addIndex(i1);
			mesh.addIndex(i2);
		}
	}
}
