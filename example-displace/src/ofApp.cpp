#include "ofApp.h"

using namespace ofxCv;

void ofApp::setup() {
	ofSetVerticalSync(true);
    img.load("face.jpg");
    imgTracker.setup();
	imgTracker.update(toCv(img));
    
    ofxDelaunay delaunay;
    
    // add main face points
    ofMesh faceMesh = imgTracker.getImageMesh();
    for(std::size_t i = 0; i < faceMesh.getNumVertices(); i++) {
        delaunay.addPoint(faceMesh.getVertex(i));
    }
    
    // add boundary face points
    float scaleFactor = 1.6;
    ofPolyline outline = imgTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE);
    glm::vec2 position = imgTracker.getPosition();
    for(std::size_t i = 0; i < outline.size(); i++) {
        glm::vec2 point = (outline[i] - position) * scaleFactor + position;
        delaunay.addPoint(point);
    }
    
    // add the image corners
    int w = img.getWidth(), h = img.getHeight();
    delaunay.addPoint(ofVec2f(0, 0));
    delaunay.addPoint(ofVec2f(w, 0));
    delaunay.addPoint(ofVec2f(w, h));
    delaunay.addPoint(ofVec2f(0, h));
    
    delaunay.triangulate();
    triangulated = delaunay.triangleMesh;
}

void ofApp::update() {
}

void ofApp::draw() {
	ofSetColor(255);
    
    // construct new mesh that has tex coords, vertices, etc.
    ofMesh finalMesh;
    finalMesh = triangulated;
    for(auto& v : triangulated.getVertices()) {
        finalMesh.addTexCoord(v.xy());
    }
    
    // modify mesh
    for(auto& v : finalMesh.getVertices()) {
        v += glm::vec3(ofRandomf(), ofRandomf(), 0) * (mouseX/10.);
    }
    
    if(ofGetKeyPressed('1')) {
        triangulated.drawWireframe();
    } else if(ofGetKeyPressed('2')) {
        finalMesh.drawWireframe();
    } else {
        img.bind();
        finalMesh.drawFaces();
        img.unbind();
    }
    
    ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
}
