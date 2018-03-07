#pragma once

#include "ofxCv.h"

class Expression {
public:
	Expression(std::string description = "");
	void setDescription(std::string description);
	std::string getDescription() const;
	void addSample(const cv::Mat& sample);
	cv::Mat& getExample(unsigned int i);
	unsigned int size() const;
	void reset();
	void save(std::string filename) const;
	void load(std::string filename);
protected:
	std::string description;
	std::vector<cv::Mat> samples;
};
