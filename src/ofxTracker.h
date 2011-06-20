#pragma once

#include "ofxCv.h"
using namespace ofxCv;
using namespace cv;

#include "Tracker.h"
using namespace FACETRACKER;

#include "ofPolyline.h"

class ofxTracker {
public:
	ofxTracker();
	void setup();
	void update(Mat image);
	void draw() const;
	void reset();
	
	int size() const;
	bool getFound() const;
	bool getVisibility(int i) const;
	ofVec2f getImagePoint(int i) const; // on the 2d screen
	ofVec3f getObjectPoint(int i) const; // in a normalized 3d space
	ofMesh getImageMesh() const; // on the 2d screen
	ofMesh getObjectMesh() const; // in a normalized 3d space
	
	ofVec2f getPosition() const;
	float getScale() const; // multiply by ~20-23 to get pixel units (+/-20 units in the x axis, +23/-18 on the y axis)
	ofVec3f getOrientation() const;
	ofMatrix4x4 getRotationMatrix() const;
	
	enum Feature {
		LEFT_EYEBROW,
		RIGHT_EYEBROW,
		LEFT_EYE,
		RIGHT_EYE,
		LEFT_JAW,
		RIGHT_JAW,
		JAW,
		OUTER_MOUTH
	};
	ofPolyline getFeature(Feature feature) const;
	
	//float getGesture() const;
	// MOUTH_WIDTH, MOUTH_HEIGHT,
	// MOUTH_WIGGLE, // mouth left/right
	// LEFT_EYEBROW, RIGHT_EYEBROW, // up/down
	// LEFT_EYE, RIGHT_EYE // open/close
	// JAW // openness
	// NOSTRILS // flaring is subtle but detectable
	// NOSE_WIGGLE // not everyone can do it
	// similar, but separate: rotation, position
	
	void setScale(float scale); // need to rename this due to getScale
	void setIterations(int iterations);
	void setClamp(float clamp);
	void setTolerance(float tolerance);
	void setAttempts(int attempts);
	
protected:
	void updateObjectPoints();

	bool failed;
	int idx;
	
	bool fcheck;
	double scale;
	int frameSkip;
	
	vector<int> wSize1;
	vector<int> wSize2;
	int iterations;
	int attempts;
	double clamp, tolerance;
	
	Tracker tracker;
	Mat tri, con;
	
	Mat im, gray;
	Mat objectPoints;
};