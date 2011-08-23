#pragma once

#include "ofxCv.h"
#include "Expression.h"

#include "ofxFaceTracker.h"
class ofxFaceTracker;

class ExpressionClassifier {
public:
	ExpressionClassifier();
	void save(string directory) const;
	void load(string directory);
	unsigned int classify(const ofxFaceTracker& tracker); // helper method
	unsigned int classify(const cv::Mat& data); // slightly more direct
	unsigned int getPrimaryExpression() const;
	double getProbability(unsigned int i) const;
	string getDescription(unsigned int i) const;
	Expression& getExpression(unsigned int i);
	void setSigma(double sigma);
	double getSigma() const;
	unsigned int size() const;
	void addExpression(string description = "");
	void addExpression(Expression& expression);
	void addSample(const ofxFaceTracker& tracker); // helper method
	void addSample(const cv::Mat& sample); // slightly more direct
	void reset();
protected:
	vector<Expression> expressions;
	vector<double> probability;
	float sigma;
};
