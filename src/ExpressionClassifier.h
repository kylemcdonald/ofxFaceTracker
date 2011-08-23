#pragma once

#include "ofxFaceTracker.h"
#include "Expression.h"

/*
	sigma describes the classification sharpness. A larger sigma means the
	boundary between different expressions is more blurry. It won't change
	the classification, but will give you probabilities that are smoother.
*/

class ExpressionClassifier {
public:
	ExpressionClassifier();
	void save(string directory) const;
	void load(string directory);
	unsigned int classify(Mat& data);
	unsigned int getPrimaryExpression() const;
	double getProbability(unsigned int i) const;
	string getDescription(unsigned int i) const;
	Expression& getExpression(unsigned int i);
	void setSigma(double sigma);
	double getSigma() const;
	unsigned int size() const;
	void addExpression(string description = "");
	void addExpression(Expression& expression);
	void addSample(Mat& sample);
	void reset();
protected:
	vector<Expression> expressions;
	vector<double> probability;
	float sigma;
};
