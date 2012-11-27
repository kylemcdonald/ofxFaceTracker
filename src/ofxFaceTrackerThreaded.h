#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxFaceTracker.h"

class ofxFaceTrackerThreaded : public ofxFaceTracker, public ofThread {
public:
	ofxFaceTrackerThreaded()
	:needsUpdatingBack(false)
	,needsUpdatingFront(false)
	,meanObjectPointsReady(false) {
	}
	~ofxFaceTrackerThreaded() {
		stopThread();
		ofSleepMillis(500);
	}
	void setup() {
		failed = true;
		ofxFaceTracker::setup();
		startThread(true, false);
	}
	bool update(cv::Mat image) {
		dataMutex.lock();
		image.copyTo(imageMiddle);
		objectPointsFront = objectPointsMiddle;
		imagePointsFront = imagePointsMiddle;
		meanObjectPointsFront = meanObjectPointsMiddle;
		objectPointsMatFront = objectPointsMatMiddle;
		failed = failedMiddle;
		needsUpdatingFront = true;
		dataMutex.unlock();
		if(!failed) {
			meanObjectPointsReady = true;
		}
		return getFound();
	}
	const cv::Mat& getObjectPointsMat() const {
		return objectPointsMatFront;
	}
	ofVec2f getImagePoint(int i) const {
		if(failed) {
			return ofVec2f();
		}
		return imagePointsFront[i];
	}
	ofVec3f getObjectPoint(int i) const {
		if(failed) {
			return ofVec3f();
		}
		return objectPointsFront[i];
	}
	ofVec3f getMeanObjectPoint(int i) const {
		if(meanObjectPointsReady) {
			return meanObjectPointsFront[i];
		} else {
			return ofVec3f();
		}
	}
	bool getVisibility(int i) const {
		return failed;
	}
	ofVec3f getOrientation() const {
		return orientation;
	}
	ofVec2f getPosition() const {
		return position;
	}
	float getScale() const {
		return scale;
	}
	
protected:
	void threadedFunction() {
		ofxFaceTracker* threadedTracker = new ofxFaceTracker();
		threadedTracker->setup();
		while(isThreadRunning()) {
			dataMutex.lock();
			needsUpdatingBack = needsUpdatingFront;
			if(needsUpdatingBack) {
				imageMiddle.copyTo(imageBack);
			}
			dataMutex.unlock();
			
			threadedTracker->setRescale(rescale);
			threadedTracker->setIterations(iterations);
			threadedTracker->setClamp(clamp);
			threadedTracker->setTolerance(tolerance);
			threadedTracker->setAttempts(attempts);
			threadedTracker->setUseInvisible(useInvisible);
			
			if(needsUpdatingBack) {
				threadedTracker->update(imageBack);
			} else {
				ofSleepMillis(4);
			}
			
			dataMutex.lock();
			objectPointsMiddle = threadedTracker->getObjectPoints();
			imagePointsMiddle = threadedTracker->getImagePoints();
			meanObjectPointsMiddle = threadedTracker->getMeanObjectPoints();
			failedMiddle = !threadedTracker->getFound();
			position = threadedTracker->getPosition();
			orientation = threadedTracker->getOrientation();
			scale = threadedTracker->getScale();
			objectPointsMatMiddle = threadedTracker->getObjectPointsMat();
			dataMutex.unlock();
		}
		delete threadedTracker;
	}
	
	ofMutex dataMutex;
	
	bool needsUpdatingBack, needsUpdatingFront;
	cv::Mat imageMiddle, imageBack;
	vector<ofVec3f> objectPointsFront, objectPointsMiddle;
	vector<ofVec2f> imagePointsFront, imagePointsMiddle;
	vector<ofVec3f> meanObjectPointsFront, meanObjectPointsMiddle;
	bool failedMiddle;
	bool meanObjectPointsReady;
	
	ofVec3f orientation;
	float scale;
	ofVec2f position;
	cv::Mat objectPointsMatBack, objectPointsMatMiddle, objectPointsMatFront; 
};
