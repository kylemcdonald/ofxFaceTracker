#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"
#include "ofxGui.h"

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
		
		// get the median at the 92nd percentile
		vector<float> all;
		all.assign(buffer.begin(), buffer.end());
		ofSort(all);
		float percentile = 0.92;
		threshold = all[(int) (all.size() * percentile)];
	}
	
	void addSample(float sample, float percentile) {
		buffer.push_back(sample);
		if(buffer.size() > maxSize) {
			buffer.pop_front();
		}
		
		// get the median at the provided percentile
		vector<float> all;
		all.assign(buffer.begin(), buffer.end());
		ofSort(all);
		threshold = all[(int) (all.size() * percentile)];
	}
	
	void glMapX(float minInput, float maxInput, float minOutput, float maxOutput) {
		float inputRange = maxInput - minInput;
		float outputRange = maxOutput - minOutput;
		ofTranslate(minOutput, 0);
		ofScale(outputRange, 1);
		ofScale(1. / inputRange, 1);
		ofTranslate(-minInput, 0);
	}
	void glMapY(float minInput, float maxInput, float minOutput, float maxOutput) {
		float inputRange = maxInput - minInput;
		float outputRange = maxOutput - minOutput;
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
            line.addVertex(glm::vec3(i, buffer[i], 0));
		}
		ofPushMatrix();
		ofPushStyle();
		
		ofRectangle box = getBounds();
		
		ofTranslate(x, y);
		ofFill();
		bool bright = !buffer.empty() && threshold != 0 && buffer.back() > threshold;
		ofSetColor(bright ? 160 : 0);
		ofDrawRectangle(0, 0, maxSize, height);
		ofNoFill();
		ofSetColor(255);
		ofDrawRectangle(0, 0, maxSize, height);
		
		if(!buffer.empty()) {
			glMapX(box.x, box.x + box.width, 0, maxSize);
			glMapY(box.y, box.y + box.height, height, 0);
			line.draw();
			ofDrawLine(0, threshold, buffer.size(), threshold);
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

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	ofVideoGrabber cam;
	ofxFaceTracker tracker;
	ofImage camImage;
	
	ofFbo eyeFbo;
	ofPixels eyePixels;
	ofxCvColorImage		eyeImageColor;
	ofxCvGrayscaleImage	eyeImageGray;
	ofxCvGrayscaleImage	eyeImageGrayPrev;
	ofxCvGrayscaleImage	eyeImageGrayDif;
	
	ofMesh leftRectImg, rightRectImg;
	Graph rowGraph;
	
	glm::vec2 position;
	ofxFloatSlider percentileThreshold;
	ofxPanel gui;
	
	float scale;
	ofMatrix4x4 rotationMatrix;
	
	ofxOscSender osc;
};
