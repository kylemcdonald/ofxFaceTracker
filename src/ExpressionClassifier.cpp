#include "ExpressionClassifier.h"

using namespace cv;


/*
	sigma describes the classification sharpness. A larger sigma means the
	boundary between different expressions is more blurry. It won't change
	the classification, but will give you probabilities that are smoother.
*/

ExpressionClassifier::ExpressionClassifier()
:sigma(10.0) {
}

void ExpressionClassifier::save(string directory) const {
	ofDirectory dir(directory);
	dir.create(true);
	for(int i = 0; i < size(); i++) {
		string filename = dir.path() + "/" + expressions[i].getDescription() + ".yml";
		cout << "saving to " << filename << endl;
		expressions[i].save(filename);
	}
}

void ExpressionClassifier::load(string directory) {
	ofDirectory dir(directory);
	dir.listDir();
	int n = dir.size();
	expressions.resize(n);
	for(int i = 0; i < n; i++) {
		expressions[i].load(dir.getPath(i));
	}
}

unsigned int ExpressionClassifier::classify(const ofxFaceTracker& tracker) {
	Mat cur;
	tracker.getObjectPointsMat().copyTo(cur);
	norm(cur);
	int n = size();
	probability.resize(n);
	if(n == 0) {
		return 0;
	}
	vector<vector<double> > val(n);
	double sum = 0;
	for(int i = 0; i < n; i++){
		int m = expressions[i].size();
		for(int j = 0; j < m; j++){
			double v = norm(cur, expressions[i].getExample(j));
			double p = exp(-v * v / sigma);
			val[i].push_back(p);
			sum += p;
		}
	}
	for(int i = 0; i < n; i++){
		probability[i] = 0;
		int m = expressions[i].size();
		for(int j = 0; j < m; j++) {
			probability[i] += val[i][j];
		}
		probability[i] /= sum;
	}
	return getPrimaryExpression();
}

unsigned int ExpressionClassifier::getPrimaryExpression() const {
	int maxExpression = 0;
	double maxProbability = 0;
	for(int i = 0; i < probability.size(); i++) {
		double cur = getProbability(i);
		if(cur > maxProbability) {
			maxExpression = i;
			maxProbability = cur;
		}
	}
	return maxExpression;
}

double ExpressionClassifier::getProbability(unsigned int i) const {
	if(i < probability.size()) {
		return probability[i];
	} else {
		return 0;
	}
}

string ExpressionClassifier::getDescription(unsigned int i) const {
	return expressions[i].getDescription();
}

Expression& ExpressionClassifier::getExpression(unsigned int i) {
	return expressions[i];
}

void ExpressionClassifier::setSigma(double sigma) {
	this->sigma = sigma;
}

double ExpressionClassifier::getSigma() const {
	return sigma;
}

unsigned int ExpressionClassifier::size() const {
	return expressions.size();
}

void ExpressionClassifier::addExpression(string description) {
	if(description == "") {
		description = ofToString(expressions.size());
	}
	expressions.push_back(Expression(description));
}

void ExpressionClassifier::addExpression(Expression& expression) {
	expressions.push_back(expression);
}

void ExpressionClassifier::addSample(const ofxFaceTracker& tracker) {
	if(size() == 0) {
		addExpression();
	}
	expressions.back().addSample(tracker.getObjectPointsMat());
}

void ExpressionClassifier::reset() {
	expressions.clear();
}