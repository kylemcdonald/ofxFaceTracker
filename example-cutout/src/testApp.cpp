#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
	ofSetVerticalSync(true);
	cam.initGrabber(640, 480);
	camTracker.setup();
	imgTracker.setup();
	
	img.loadImage("face.jpg");
	imgTracker.update(toCv(img));
}

void testApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		camTracker.update(toCv(cam));
	}
}

void testApp::draw() {
	ofSetColor(255);
	cam.draw(0, 0);
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
	
	if(camTracker.getFound()) {
		camTracker.draw();
	}
	
	ofTranslate(0, 480);
	
	if(imgTracker.getFound()) {
		ofMesh faceMesh = imgTracker.getImageMesh();
		
		ofxDelaunay delaunay;
		
		// add main face points
		for(int i = 0; i < faceMesh.getNumVertices(); i++) {
			delaunay.addPoint(faceMesh.getVertex(i));
		}
		
		// add boundary face points
		float scaleFactor = 1.6;
		ofPolyline outline = imgTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE);
		ofVec2f position = imgTracker.getPosition();
		for(int i = 0; i < outline.size(); i++) {
			ofVec2f point((outline[i] - position) * scaleFactor + position);
			delaunay.addPoint(point);
		}

		// add the image corners
		int w = img.getWidth(), h = img.getHeight();
		delaunay.addPoint(ofVec2f(0, 0));
		delaunay.addPoint(ofVec2f(w, 0));
		delaunay.addPoint(ofVec2f(w, h));
		delaunay.addPoint(ofVec2f(0, h));
		
		delaunay.triangulate();
		ofMesh triangulated = delaunay.triangleMesh;
		triangulated.drawWireframe();
		
		// find mapping between triangulated mesh and original
		vector<int> delaunayToFinal(triangulated.getNumVertices(), -1);
		vector<int> finalToDelaunay;
		int reindexed = 0;
		for(int i = 0; i < faceMesh.getNumVertices(); i++) {
			float minDistance = 0;
			int best = 0;
			for(int j = 0; j < triangulated.getNumVertices(); j++) {
				float distance = triangulated.getVertex(j).distance(faceMesh.getVertex(i));
				if(j == 0 || distance < minDistance) {
					minDistance = distance;
					best = j;
				}
			}
			delaunayToFinal[best] = reindexed++;
			finalToDelaunay.push_back(best);
		}
		for(int i = 0; i < delaunayToFinal.size(); i++) {
			if(delaunayToFinal[i] == -1) {
				delaunayToFinal[i] = reindexed++;
				finalToDelaunay.push_back(i);
			}
		}
		
		// construct new mesh that has tex coords, vertices, etc.
		ofMesh finalMesh;
		finalMesh.setMode(OF_PRIMITIVE_TRIANGLES);	
		for(int i = 0; i < delaunayToFinal.size(); i++) {
			int index = finalToDelaunay[i];
			finalMesh.addVertex(triangulated.getVertex(index));
			finalMesh.addTexCoord(triangulated.getVertex(index));
		}
		for(int i = 0; i < triangulated.getNumIndices(); i++) {
			finalMesh.addIndex(delaunayToFinal[triangulated.getIndex(i)]);
		}
		
		// modify mesh
		if(camTracker.getFound()) {
			ofVec2f imgPosition = imgTracker.getPosition();
			ofVec2f camPosition = camTracker.getPosition();
			float imgScale = imgTracker.getScale();
			float camScale = camTracker.getScale();
			ofMesh reference = camTracker.getImageMesh();
			for(int i = 0; i < reference.getNumVertices(); i++) {
				ofVec2f point = reference.getVertices()[i];
				point -= camPosition;
				point /= camScale;
				point *= imgScale;
				point += imgPosition;
				finalMesh.getVertices()[i] = point;
			}
		}
		 
		img.bind();
		finalMesh.drawFaces();
		img.unbind();
	}
}