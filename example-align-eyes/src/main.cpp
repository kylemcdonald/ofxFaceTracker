#include "ofApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 128, 128, OF_WINDOW);
	ofRunApp(new ofApp());
}
