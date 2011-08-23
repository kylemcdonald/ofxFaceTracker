#include "Expression.h"

using namespace cv;

Expression::Expression(string description) {
	this->description = description;
}

void Expression::setDescription(string description) {
	this->description = description;
}

string Expression::getDescription() const {
	return description;
}

void Expression::addSample(const Mat& sample) {
	samples.push_back(sample.clone());
}

Mat& Expression::getExample(unsigned int i) {
	return samples[i];
}

unsigned int Expression::size() const {
	return samples.size();
}

void Expression::reset() {
	samples.clear();
}

void Expression::save(string filename) const {
	FileStorage fs(ofToDataPath(filename), FileStorage::WRITE);
	fs <<	"description" << description <<
	"samples" << "[";
	for(int i = 0; i < size(); i++) {
		fs << samples[i];
	}
	fs << "]";
}

void Expression::load(string filename) {
	FileStorage fs(ofToDataPath(filename), FileStorage::READ);
	description = (string) fs["description"];
	FileNode samplesNode = fs["samples"];
	int n = samplesNode.size();
	samples.resize(n);
	for(int i = 0; i < n; i++) {
		samplesNode[i] >> samples[i];
	}
}