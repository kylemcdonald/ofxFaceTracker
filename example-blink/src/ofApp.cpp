#include "ofApp.h"

//===================================================================
// Used to extract eye rectangles. from ProCamToolkit
GLdouble modelviewMatrix[16], projectionMatrix[16];
GLint viewport[4];
void updateProjectionState() {
	glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
	glGetIntegerv(GL_VIEWPORT, viewport);
}

glm::vec3 ofWorldToScreen(glm::vec3 world) {
	updateProjectionState();
    glm::vec3 screen = glm::project(glm::dvec3(world),
                                    glm::make_mat4(modelviewMatrix),
                                    glm::make_mat4(projectionMatrix),
                                    glm::make_vec4(viewport));
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
void addTexCoords(ofMesh& to, const std::vector<T>& from) {
    for(std::size_t i = 0; i < from.size(); i++) {
        to.addTexCoord(glm::vec2(from[i].x, from[i].y));
	}
}


//===================================================================
using namespace ofxCv;

void ofApp::setup() {
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
	cam.initGrabber(640, 480);
	
	// Set up images we'll use for frame-differencing
	camImage.allocate(640, 480, OF_IMAGE_COLOR);
	eyeImageColor.allocate(128, 48);
	eyeImageGray.allocate(128,48);
	eyeImageGrayPrev.allocate(128,48);
	eyeImageGrayDif.allocate(128,48);
	eyeFbo.allocate(128, 48, GL_RGB);
	
	// Initialize our images to black.
	eyeImageColor.set(0);
	eyeImageGray.set(0);
	eyeImageGrayPrev.set(0);
	eyeImageGrayDif.set(0);

	// Set up other objects.
	tracker.setup();
	osc.setup("localhost", 8338);
	
	// This GUI slider adjusts the sensitivity of the blink detector.
	gui.setup();
	gui.add(percentileThreshold.setup("Threshold", 0.92, 0.75, 1.0));
}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		camImage.setFromPixels(cam.getPixels());
		
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

            glm::vec2 leftCenter = leftEye.getBoundingBox().getCenter();
            glm::vec2 rightCenter = rightEye.getBoundingBox().getCenter();

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

			// Copy the extracted quadrilaterals into the eyeFbo
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
			
			// Convert the combined eye-image to grayscale,
			// and put its data into a frame-differencer.
			eyeImageColor.setFromPixels(eyePixels);
			eyeImageGrayPrev.setFromPixels(eyeImageGray.getPixels());
			eyeImageGray.setFromColorImage(eyeImageColor);
			eyeImageGray.contrastStretch();
			eyeImageGrayDif.absDiff(eyeImageGrayPrev, eyeImageGray);
			
			// Compute the average brightness of the difference image.
			unsigned char* difPixels = eyeImageGrayDif.getPixels().getData();
			int nPixels = 128*48;
			float sum = 0;
			for (int i=0; i<nPixels; i++){
				if (difPixels[i] > 4){ // don't consider diffs less than 4 gray levels;
					sum += difPixels[i]; // reduces noise
				}
			}
			// Store the current average in the row graph
			float avg = sum / (float) nPixels;
			rowGraph.addSample(avg, percentileThreshold);

			// Send out an OSC message,
			// With the value 1 if the current average is above threshold
			ofxOscMessage msg;
			msg.setAddress("/gesture/blink");
			int oscMsgInt = (rowGraph.getState() ? 1 : 0);
			msg.addIntArg(oscMsgInt);
			osc.sendMessage(msg);
			
			// Print out a message to the console if there was a blink.
			if (oscMsgInt > 0){
				printf("Blink happened at frame #:	%d\n", (int)ofGetFrameNum());
			}
		
		}
	}
}

void ofApp::draw() {
	ofSetColor(255);
	
	camImage.draw(0, 0);
	tracker.draw();
	leftRectImg.draw();
	rightRectImg.draw();
	
	float y = 58;
	gui.draw();
	eyeImageGray.draw(10, y);    y+=58;
	eyeImageGrayDif.draw(10, y); y+=58;
	rowGraph.draw(10, y, 48);
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, ofGetHeight() - 20);
}

void ofApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}
