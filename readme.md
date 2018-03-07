# ofxFaceTracker is openFrameworks addon for face tracking, based on Jason Saragih's FaceTracker library.

[openFrameworks](http://openFrameworks.cc/) is an open source toolkit for creative coding.

All ofxFaceTracker code is available under the MIT license, while FaceTracker is provided free for non-commercial use. For commercial use of FaceTracker, please [request a quote](http://facetracker.net/quote/).

## Installation

Required addons:
  - [ofxCv](https://github.com/kylemcdonald/ofxCv)
  - [ofxTiming](https://github.com/kylemcdonald/ofxTiming) (for some examples)
  - [ofxDelauny](https://github.com/obviousjim/ofxDelaunay) (for some examples)

First, download [ofxCv](hhttps://github.com/kylemcdonald/ofxCv).

Then, you need to make a copy of the `libs/FaceTracker/model/` directory in `example-*/bin/data/model/` of each example. You can do this by hand, or `python setup.py` will take care of this for you.

Then you can generate project files with the OF project generator and run the examples. If you see the error `Assertion failed: s.is_open()` when running your app, that means you forgot to drop the model files in the right directory.

If you would like to prototype an idea involving face tracking, I encourage you to download [FaceOSC](https://github.com/kylemcdonald/ofxFaceTracker/releases). Dan Wilcox has some great [FaceOSC templates](hhttps://github.com/CreativeInquiry/FaceOSC-Templates) that will help you get started in environments like Max, pd, Processing, and openFrameworks.

If you're interested in using ofxFaceTracker for face substitution, check out the [FaceSubstitution](https://github.com/arturoc/FaceSubstitution) repository.

## Examples

### iOS

Getting the combination of ofxFaceTracker and ofxCv working on iOS can be complicated and confusing. Try using this [ofxFaceTracker-iOS repo](https://github.com/kylemcdonald/ofxFaceTracker-iOS) as a starting point.

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


## FaceOSC

FaceOSC sends as much data as possible from `ofxFaceTracker` via OSC, including its pose/position and gestural data, and its raw points. It also streams the image over [Syphon](http://syphon.v002.info/).

You can download a pre-built binary in the [releases page](https://github.com/kylemcdonald/ofxFaceTracker/releases).   

#### OSC information

 * Pose
  * **center position**: /pose/position
  * **scale**: /pose/scale
  * **orientation** (which direction you're facing): /pose/orientation
 * Gestures
  * **mouth width**: /gesture/mouth/width
  * **mouth height**: /gesture/mouth/height
  * **left eyebrow height**: /gesture/eyebrow/left
  * **right eyebrow height**: /gesture/eyebrow/right
  * **left eye openness**: /gesture/eye/left
  * **right eye openness**: /gesture/eye/right
  * **jaw openness**: /gesture/jaw
  * **nostril flate**: /gesture/nostrils
 * Raw
  * **raw points** (66 xy-pairs): /raw


