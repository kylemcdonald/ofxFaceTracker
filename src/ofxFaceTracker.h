#pragma once

#include "ofxCv.h"
using namespace ofxCv;
using namespace cv;

#include "Tracker.h"
using namespace FACETRACKER;

#include "ofPolyline.h"

class ofxFaceTracker {
public:
	ofxFaceTracker();
	void setup();
	void update(Mat image);
	void draw() const;
	void reset();
	
	int size() const;
	bool getFound() const;
	bool getVisibility(int i) const;
	ofVec2f getImagePoint(int i) const; // on the 2d screen
	ofVec3f getObjectPoint(int i) const; // in a normalized 3d space
	ofMesh & getImageMesh() const; // on the 2d screen
	ofMesh & getObjectMesh() const; // in a normalized 3d space
	
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
		// INNER_MOUTH // needs a little more love
	};
	ofPolyline getFeature(Feature feature) const;
	
	enum Gesture {
		MOUTH_WIDTH, MOUTH_HEIGHT,
		LEFT_EYEBROW_HEIGHT, RIGHT_EYEBROW_HEIGHT,
		LEFT_EYE_OPENNESS, RIGHT_EYE_OPENNESS,
		JAW_OPENNESS,
		NOSTRIL_FLARE
		// MOUTH_WIGGLE // left/right
		// NOSE_WIGGLE // left/right
	};
	float getGesture(Gesture gesture) const;
	
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

	ofMesh imageMesh, objectMesh;
	bool imageMeshDirty, objectMeshDirty;
};
