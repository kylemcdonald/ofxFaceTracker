#include "testApp.h"

#include "ofMeshUtils.h"

using namespace ofxCv;
using namespace cv;

void testApp::setup() {
	ofSetVerticalSync(true);
	
	cam.initGrabber(640, 480);
	tracker.setup();
	calibration.load("mbp-isight.yml");
	
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	light.setPosition(-500, 0, 0);
}

void testApp::updatePhysicalMesh() {
	// 1 load object and image points as Point2f/3f
	vector<Point3f> objectPoints;
	vector<Point2f> imagePoints;
	for(int i = 0; i < tracker.size(); i++) {
		objectPoints.push_back(toCv(tracker.getObjectPoint(i)));
		imagePoints.push_back(toCv(tracker.getImagePoint(i)));
	}
	
	// 2 guess for the rotation and translation of the face
	Mat cameraMatrix = calibration.getDistortedIntrinsics().getCameraMatrix();
	Mat distCoeffs = calibration.getDistCoeffs();
	Mat rvec, tvec;
	solvePnP(Mat(objectPoints),  Mat(imagePoints),
					 cameraMatrix,  distCoeffs,
					 rvec, tvec);
	
	// 3 reproject using guess, and fit to the actual image location
	vector<ofVec3f> fitWorldPoints;
	Mat cameraMatrixInv = cameraMatrix.inv();
	Mat rmat;
	Rodrigues(rvec, rmat);
	for(int i = 0; i < objectPoints.size(); i++) {
		Point2d imgImg = imagePoints[i];
		Point3d objObj = objectPoints[i];
		Point3d imgHom(imgImg.x, imgImg.y, 1.); // img->hom
		Point3d imgWor = (Point3f) Mat(cameraMatrixInv * Mat(imgHom)); // hom->wor
		Point3d objWor = (Point3d) Mat(tvec + rmat * Mat(objObj)); // obj->wor
		Point3d fitWor = intersectPointRay(objWor, imgWor); // scoot it over
		// if it was projected on the wrong side, flip it over
		if(fitWor.z < 0) {
			fitWor *= -1;
		}
		fitWorldPoints.push_back(toOf(fitWor));
		// convert down to image space coordinates
		//Point3d fitHom = (Point3d) Mat(cameraMatrix * Mat(fitWor)); // wor->hom
		//Point2d fitImg(fitHom.x / fitHom.z, fitHom.y / fitHom.z); // hom->img
	}
	
	// 4 use the resulting 3d points to build a mesh with normals
	physicalMesh = convertFromIndices(tracker.getMesh(fitWorldPoints));
	physicalMesh.setMode(OF_PRIMITIVE_TRIANGLES);
	buildNormals(physicalMesh);
}

void testApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		tracker.update(toCv(cam));
		if(tracker.getFound()) {
			updatePhysicalMesh();
		}
	}
}

void testApp::draw() {
	ofSetColor(255);
	cam.draw(0, 0);
	
	if(tracker.getFound()) {
		calibration.getDistortedIntrinsics().loadProjectionMatrix();
		ofEnableLighting();
		light.enable();
		physicalMesh.drawFaces();
		ofDisableLighting();
	}
}

void testApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}