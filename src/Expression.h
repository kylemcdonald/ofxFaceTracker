#pragma once

#include "ofxCv.h"

class Expression {
public:
	Expression(string description = "");
	void setDescription(string description);
	string getDescription() const;
	void addSample(const cv::Mat& sample);
	cv::Mat& getExample(unsigned int i);
	unsigned int size() const;
	void reset();
	void save(string filename) const;
	void load(string filename);
protected:
	string description;
	vector<cv::Mat> samples;
};