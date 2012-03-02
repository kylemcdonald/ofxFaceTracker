#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOsc.h"

#include "ofxFaceTracker.h"

class Graph {
public:
	Graph()
	:maxSize(128)
	,threshold(0)
	,state(false) {
	}
	void setup(int maxSize) {
		this->maxSize = maxSize;
	}
	void addSample(float sample) {
		buffer.push_back(sample);
		if(buffer.size() > maxSize) {
			buffer.pop_front();
		}
		
		// get the median at the 95th percentile
		vector<float> all;
		all.assign(buffer.begin(), buffer.end());
		ofSort(all);
		float percentile = .95;
		threshold = all[(int) (all.size() * percentile)];
	}
	void glMapX(float minInput, float maxInput, float minOutput, float maxOutput) {
		float inputRange = maxInput - minInput, outputRange = maxOutput - minOutput;
		ofTranslate(minOutput, 0);
		ofScale(outputRange, 1);
		ofScale(1. / inputRange, 1);
		ofTranslate(-minInput, 0);
	}
	void glMapY(float minInput, float maxInput, float minOutput, float maxOutput) {
		float inputRange = maxInput - minInput, outputRange = maxOutput - minOutput;
		ofTranslate(0, minOutput);
		ofScale(1, outputRange);
		ofScale(1, 1. / inputRange);
		ofTranslate(0, -minInput);
	}
	ofRectangle getBounds() {
		return line.getBoundingBox();
	}
	void draw(int x, int y, int height) {
		line.clear();
		for(int i = 0; i < buffer.size(); i++) {
			line.addVertex(ofVec2f(i, buffer[i]));
		}
		ofPushMatrix();
		ofPushStyle();
		
		ofRectangle box = getBounds();
		
		ofTranslate(x, y);
		ofFill();
		bool bright = !buffer.empty() && threshold != 0 && buffer.back() > threshold;
		ofSetColor(bright ? 128 : 0);
		ofRect(0, 0, maxSize, height);
		ofNoFill();
		ofSetColor(255);
		ofRect(0, 0, maxSize, height);
		
		if(!buffer.empty()) {
			glMapX(box.x, box.x + box.width, 0, maxSize);
			glMapY(box.y, box.y + box.height, height, 0);
			line.draw();
			ofLine(0, threshold, buffer.size(), threshold);
			state = buffer.back() > threshold;
		}
		ofPopStyle();
		ofPopMatrix();
	}
	bool getState() {
		return state;
	}
private:
	ofPolyline line;
	int maxSize;
	deque<float> buffer;
	float threshold;
	bool state;
};

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	ofVideoGrabber cam;
	ofxFaceTracker tracker;
	
	ofFbo eyeFbo;
	ofPixels eyePixels;
	ofImage sobelImg;
	cv::Mat grayFloat;
	cv::Mat sobelx, sobely;
	cv::Mat rowMean, gray, sobel;
	ofPolyline rowMeanLine;
	ofMesh leftRectImg, rightRectImg;

	float runningMean;
	Graph rowGraph;
	
	ofVec2f position;
	float scale;
	ofMatrix4x4 rotationMatrix;
	
	ofxOscSender osc;
};
