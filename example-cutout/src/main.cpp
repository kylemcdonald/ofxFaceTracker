#include "ofApp.h"


int main() {
    ofSetupOpenGL(640, 960, OF_WINDOW);
    return ofRunApp(std::make_shared<ofApp>());
}
