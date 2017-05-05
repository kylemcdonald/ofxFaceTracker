#include "ofApp.h"

// from ProCamToolkit
GLdouble modelviewMatrix[16], projectionMatrix[16];
GLint viewport[4];
void updateProjectionState() {
	glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
	glGetIntegerv(GL_VIEWPORT, viewport);
}

glm::vec3 ofWorldToScreen(glm::vec3 world) {
	updateProjectionState();
	GLdouble x, y, z;
	gluProject(world.x, world.y, world.z, modelviewMatrix, projectionMatrix, viewport, &x, &y, &z);
	glm::vec3 screen(x, y, z);
	screen.y = ofGetHeight() - screen.y;
	return screen;
}

ofMesh getProjectedMesh(const ofMesh& mesh) {
	ofMesh projected = mesh;
    for(std::size_t i = 0; i < mesh.getNumVertices(); i++) {
		glm::vec3 cur = ofWorldToScreen(mesh.getVerticesPointer()[i]);
		cur.z = 0;
		projected.setVertex(i, cur);
	}
	return projected;
}

template <class T>
void addTexCoords(ofMesh& to, const vector<T>& from) {
    for(std::size_t i = 0; i < from.size(); i++) {
        to.addTexCoord(glm::vec2(from[i].x, from[i].y));
	}
}

using namespace ofxCv;

void ofApp::setup() {
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
	cam.initGrabber(640, 480);

	tracker.setup();
	eyeFbo.allocate(128, 48, GL_RGB);
	runningMean = 24;

	osc.setup("localhost", 8338);
}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		tracker.update(toCv(cam));
		position = tracker.getPosition();
		scale = tracker.getScale();
		rotationMatrix = tracker.getRotationMatrix();

		if(tracker.getFound()) {
			glm::vec2
			leftOuter = tracker.getImagePoint(36),
			leftInner = tracker.getImagePoint(39),
			rightInner = tracker.getImagePoint(42),
			rightOuter = tracker.getImagePoint(45);

			ofPolyline leftEye = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE);
			ofPolyline rightEye = tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE);

            glm::vec2 leftCenter = leftEye.getBoundingBox().getCenter().xy();
			glm::vec2 rightCenter = rightEye.getBoundingBox().getCenter().xy();

            float leftRadius = (glm::distance(leftCenter, leftInner) + glm::distance(leftCenter, leftOuter)) / 2;
            float rightRadius = (glm::distance(rightCenter, rightInner) + glm::distance(rightCenter, rightOuter)) / 2;

			glm::vec3
			leftOuterObj = tracker.getObjectPoint(36),
			leftInnerObj = tracker.getObjectPoint(39),
			rightInnerObj = tracker.getObjectPoint(42),
			rightOuterObj = tracker.getObjectPoint(45);

			glm::vec3 upperBorder(0, -3.5, 0), lowerBorder(0, 2.5, 0);
			glm::vec3 leftDirection(-1, 0, 0), rightDirection(+1, 0, 0);
			float innerBorder = 1.5, outerBorder = 2.5;

			ofMesh leftRect, rightRect;
			leftRect.setMode(OF_PRIMITIVE_LINE_LOOP);
			leftRect.addVertex(leftOuterObj + upperBorder + leftDirection * outerBorder);
			leftRect.addVertex(leftInnerObj + upperBorder + rightDirection * innerBorder);
			leftRect.addVertex(leftInnerObj + lowerBorder + rightDirection * innerBorder);
			leftRect.addVertex(leftOuterObj + lowerBorder + leftDirection * outerBorder);
			rightRect.setMode(OF_PRIMITIVE_LINE_LOOP);
			rightRect.addVertex(rightInnerObj+ upperBorder + leftDirection * innerBorder);
			rightRect.addVertex(rightOuterObj + upperBorder + rightDirection * outerBorder);
			rightRect.addVertex(rightOuterObj + lowerBorder + rightDirection * outerBorder);
			rightRect.addVertex(rightInnerObj + lowerBorder + leftDirection * innerBorder);

			ofPushMatrix();
			ofSetupScreenOrtho(640, 480, -1000, 1000);
			ofScale(1, 1, -1);
			ofTranslate(position);
			applyMatrix(rotationMatrix);
			ofScale(scale, scale, scale);
			leftRectImg = getProjectedMesh(leftRect);
			rightRectImg = getProjectedMesh(rightRect);
			ofPopMatrix();

			// more effective than using object space points would be to use image space
			// but translate to the center of the eye and orient the rectangle in the
			// direction the face is facing.
			/*
			 ofPushMatrix();
			 ofTranslate(tracker.getImageFeature(ofxFaceTracker::LEFT_EYE).getCentroid2D());
			 applyMatrix(rotationMatrix);
			 ofRect(-50, -40, 2*50, 2*40);
			 ofPopMatrix();

			 ofPushMatrix();
			 ofTranslate(tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE).getCentroid2D());
			 applyMatrix(rotationMatrix);
			 ofRect(-50, -40, 2*50, 2*40);
			 ofPopMatrix();
			 */

			ofMesh normRect, normLeft, normRight;
			normRect.addVertex(glm::vec3(0, 0, 0));
			normRect.addVertex(glm::vec3(64, 0, 0));
			normRect.addVertex(glm::vec3(64, 48, 0));
			normRect.addVertex(glm::vec3(0, 48, 0));
			normLeft.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
			normRight.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
			normLeft.addVertices(normRect.getVertices());
			normRight.addVertices(normRect.getVertices());
			addTexCoords(normLeft, leftRectImg.getVertices());
			addTexCoords(normRight, rightRectImg.getVertices());

			eyeFbo.begin();
			ofSetColor(255);
			ofFill();
			cam.getTexture().bind();
			normLeft.draw();
			ofTranslate(64, 0);
			normRight.draw();
			cam.getTexture().unbind();
			eyeFbo.end();
			eyeFbo.readToPixels(eyePixels);

			convertColor(eyePixels, gray, CV_RGB2GRAY);
			normalize(gray, gray);
			Sobel(gray, sobelx, CV_32F, 1, 0, 3, 1);
			Sobel(gray, sobely, CV_32F, 0, 1, 3, 1);
			sobel = abs(sobelx) + abs(sobely);
			bitwise_not(gray, gray);
			gray.convertTo(grayFloat, CV_32F);
			sobel += gray;

			rowMean = meanRows(sobel);
			// clear the ends
			rowMean.at<float>(0) = 0;
			rowMean.at<float>(rowMean.rows - 1) = 0;
			// build the line
			rowMeanLine.clear();
			float avg = 0, sum = 0;
			for(int i = 0; i < rowMean.rows; i++) {
				float cur = rowMean.at<float>(i);
				avg += i * cur;
				sum += cur;
				rowMeanLine.addVertex(cur, i);
			}
			avg /= sum;
			rowGraph.addSample(avg - runningMean);
			runningMean = 0;//ofLerp(runningMean, avg, .3);

			cv::Mat sobelImgMat = toCv(sobelImg);
			imitate(sobelImg, gray);
			sobel.convertTo(sobelImgMat, CV_8U, .5);
			sobelImg.update();

			ofxOscMessage msg;
			msg.setAddress("/gesture/blink");
			msg.addIntArg(rowGraph.getState() ? 1 : 0);
			osc.sendMessage(msg);
		}
	}
}

void ofApp::draw() {
	ofSetColor(255);
	cam.draw(0, 0);
	tracker.draw();
	leftRectImg.draw();
	rightRectImg.draw();
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, ofGetHeight() - 20);

	ofTranslate(10, 10);
	eyeFbo.draw(0, 0);

	ofTranslate(0, 48 + 10);
	sobelImg.draw(0, 0);

	ofNoFill();
	ofPushMatrix();
	ofTranslate(128, 0);
	ofScale(.3, 1);
	rowMeanLine.draw();
	ofPopMatrix();

	ofTranslate(0, 48 + 10);
	rowGraph.draw(0, 0, 64);
}

void ofApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}
