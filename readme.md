# ofxFaceTracker is openFrameworks addon for face tracking, based on Jason Saragih's FaceTracker library.

[openFrameworks](http://openFrameworks.cc/) is an open source toolkit for creative coding.

All ofxFaceTracker code is available under the MIT license, while FaceTracker is provided free for non-commercial use. For commercial use, please contact the author of FaceTracker, [Jason Saragih](mailto:jason.saragih@csiro.au).

After downloading or cloning ofxFaceTracker, you need to make a copy of the `/libs/Tracker/model/` directory in `bin/data/model/` of each example. You can do this by hand, or `python update-projects.py` will take care of this for you. If you see the error `Assertion failed: s.is_open()`, that means you forgot to drop the model files in the right directory.

If you would like to prototype an idea involving face tracking, I encourage you to download [FaceOSC](https://github.com/kylemcdonald/ofxFaceTracker/downloads). Dan Wilcox has some great [FaceOSC templates](https://github.com/danomatika/FaceOSC-Templates) that will help you get started in environments like Max, pd, Processing, and openFrameworks.

If you're interested in using ofxFaceTracker for face substitution, check out the [FaceSubstitution](https://github.com/arturoc/FaceSubstitution) repository.

## Examples

### Advanced

Demonstrates how to get the image-space position of the face, and the 3d orientation (collectively, the face "pose") then applies these to the OpenGL context in order to draw an oriented face mesh.

### Blink

An advanced example that shows how you might detect blinking from the extracted eye images. This is not necessarily a robust classifier for whether eyes are open or closed, but works well for blinks as events.

### Calibrated

Fairly complicated example that demonstrates some advanced/experimental features of ofxCv and ofxFaceTracker for doing AR-style augmentation of the face.

### Empty

Provides a minimal example of using an `ofxVideoGrabber` with an `ofxFaceTracker`, then extracts and draws the `ofPolyline` representing the  left and right eyes.

### Expression

Demonstrates the `ExpressionClassifier`, which can load, save, and classify expressions into pre-trained categories. Provides basic expressions (eyebrows raised, neutral, smiling) as examples.

### Extraction

Demonstrates how to use the mean face mesh to draw pose and expression normalized representation of the face currently being tracked.

### FaceOSC

Sends as much data as possible from `ofxFaceTracker` via OSC, including various "gestures", position, and orientation.