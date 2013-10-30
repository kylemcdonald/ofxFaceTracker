/*
 ofxFaceTracker provides an interface to Jason Saragih's FaceTracker library.
 
 getImagePoint()/getImageMesh() are in image space. This means that all the
 points will line up with the pixel coordinates of the image you fed into
 ofxFaceTracker.
 
 getObjectPoint()/getObjectMesh() are in 3d object space. This is a product of
 the mean mesh with only the expression applied. There is no rotation or
 translation applied to the object space.
 
 getMeanObjectPoint()/getMeanObjectMesh() are also in 3d object space. However,
 there is no expression applied to the mesh.
 */

#pragma once

#include "ofxCv.h"
#include "FaceTracker/Tracker.h"
#include "ExpressionClassifier.h"

class ofxFaceTracker {
public:
	ofxFaceTracker();
	void setup();
	virtual bool update(cv::Mat image);
	void draw(bool drawLabels = false) const;
	void reset();
	
	int size() const;
	bool getFound() const;
	bool getHaarFound() const;
	int getAge() const;
	virtual bool getVisibility(int i) const;
	
	vector<ofVec2f> getImagePoints() const;
	vector<ofVec3f> getObjectPoints() const;
	vector<ofVec3f> getMeanObjectPoints() const;
	
	virtual ofVec2f getImagePoint(int i) const;
	virtual ofVec3f getObjectPoint(int i) const;
	virtual ofVec3f getMeanObjectPoint(int i) const;
	
	ofMesh getImageMesh() const;
	ofMesh getObjectMesh() const;
	ofMesh getMeanObjectMesh() const;
	template <class T> ofMesh getMesh(vector<T> points) const;
	
	virtual const cv::Mat& getObjectPointsMat() const;
	
	virtual ofRectangle getHaarRectangle() const;
	virtual ofVec2f getPosition() const;
	virtual float getScale() const;
	virtual ofVec3f getOrientation() const;
	ofMatrix4x4 getRotationMatrix() const;
	
	enum Direction {
		FACING_FORWARD,
		FACING_LEFT, FACING_RIGHT,
		FACING_UNKNOWN
	};
	Direction getDirection() const;
	
	enum Feature {
		LEFT_EYEBROW, RIGHT_EYEBROW,
		LEFT_EYE, RIGHT_EYE,
		LEFT_JAW, RIGHT_JAW, JAW,
		OUTER_MOUTH, INNER_MOUTH,
		NOSE_BRIDGE, NOSE_BASE,
		FACE_OUTLINE, ALL_FEATURES
	};
	ofPolyline getImageFeature(Feature feature) const;
	ofPolyline getObjectFeature(Feature feature) const;
	ofPolyline getMeanObjectFeature(Feature feature) const;
	
	enum Gesture {
		MOUTH_WIDTH, MOUTH_HEIGHT,
		LEFT_EYEBROW_HEIGHT, RIGHT_EYEBROW_HEIGHT,
		LEFT_EYE_OPENNESS, RIGHT_EYE_OPENNESS,
		JAW_OPENNESS,
		NOSTRIL_FLARE
	};
	float getGesture(Gesture gesture) const;
	
	void setRescale(float rescale);
	void setIterations(int iterations);
	void setClamp(float clamp);
	void setTolerance(float tolerance);
	void setAttempts(int attempts);
	void setUseInvisible(bool useInvisible);
	void setHaarMinSize(float minSize);
	
protected:
	void updateObjectPoints();
	void addTriangleIndices(ofMesh& mesh) const;
	static vector<int> getFeatureIndices(Feature feature);
	template <class T> ofPolyline getFeature(Feature feature, vector<T> points) const;
	
	bool failed;
	int age;
	int currentView;
	
	bool fcheck;
	double rescale;
	int frameSkip;
	
	vector<int> wSize1, wSize2;
	int iterations;
	int attempts;
	double clamp, tolerance;
	bool useInvisible;
	
	FACETRACKER::Tracker tracker;
	cv::Mat tri, con;
	
	cv::Mat im, gray;
	cv::Mat objectPoints;
};

template <class T>
ofPolyline ofxFaceTracker::getFeature(Feature feature, vector<T> points) const {
	ofPolyline polyline;
	if(!failed) {
		vector<int> indices = getFeatureIndices(feature);
		for(int i = 0; i < indices.size(); i++) {
			int cur = indices[i];
			if(useInvisible || getVisibility(cur)) {
				polyline.addVertex(points[cur]);
			}
		}
		switch(feature) {
			case LEFT_EYE:
			case RIGHT_EYE:
			case OUTER_MOUTH:
			case INNER_MOUTH:
			case FACE_OUTLINE:
				polyline.close();
		}
	}
	return polyline;
}

template <class T>
ofMesh ofxFaceTracker::getMesh(vector<T> points) const {
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	if(!failed) {
		int n = size();
		for(int i = 0; i < n; i++) {
			mesh.addVertex(points[i]);
			mesh.addTexCoord(getImagePoint(i));
		}
		addTriangleIndices(mesh);
	}
	return mesh;
}
