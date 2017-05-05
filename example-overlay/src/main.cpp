#include "ofApp.h"


int main() {
    ofSetupOpenGL(1280, 720, OF_WINDOW);
    return ofRunApp(std::make_shared<ofApp>());
}
