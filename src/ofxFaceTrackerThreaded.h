#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxFaceTracker.h"

class ofxFaceTrackerThreaded : public ofxFaceTracker, public ofThread {
public:
	ofxFaceTrackerThreaded()
	:needsUpdatingBack(false)
	,needsUpdatingFront(false)
	,needsResetting(false)
	,meanObjectPointsReady(false) {
	}
	~ofxFaceTrackerThreaded() {
		if(isThreadRunning()) {
            ofLog(OF_LOG_ERROR, "ofxFaceTrackerThreaded :: Thread was not stopped. You must call the trackers waitForThread() in ofApp::exit() or exit() of class that holds this object.");
        }
	}
	void setup() {
		failed = true;
        failedMiddle = true;
		ofxFaceTracker::setup();
		startThread(true);
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
		if(failed || imagePointsFront.size() == 0) {
			return ofVec2f();
		}
		return imagePointsFront[i];
	}
	ofVec3f getObjectPoint(int i) const {
		if(failed || objectPointsFront.size() == 0) {
			return ofVec3f();
		}
		return objectPointsFront[i];
	}
	ofVec3f getMeanObjectPoint(int i) const {
		if(meanObjectPointsReady || meanObjectPointsFront.size() != 0) {
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
    void reset() {
        needsResetting = true;
    }
	
protected:
	void threadedFunction() {
		ofxFaceTracker* threadedTracker = new ofxFaceTracker();
		threadedTracker->setup();
        if(!isThreadRunning()) {
            ofLog(OF_LOG_ERROR, "ofxFaceTrackerThreaded :: Error Failed to start thread.");
        }
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
      threadedTracker->setHaarMinSize(tracker._fdet._min_size);
			
			if(needsResetting){
                threadedTracker->ofxFaceTracker::reset();
				needsResetting = false;
			} else if(needsUpdatingBack) {
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
	
	bool needsUpdatingBack, needsUpdatingFront, needsResetting;
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
