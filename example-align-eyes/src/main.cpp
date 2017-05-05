#include "ofApp.h"


int main() {
    ofSetupOpenGL(128, 128, OF_WINDOW);
    return ofRunApp(std::make_shared<ofApp>());
}
