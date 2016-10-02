
Settings
--------

The settings for FaceOSC are are found in the settings.xml file located at: 
* Mac OSX: right click on FaceOSC.app, select "Show Package Contents", and navigate to Contents/Resources/data/ 
* Win & Linux: included in the FaceOSC folder

Further instructions are contained within the settings.xml file.

Playing Movies
--------------

Put the movie file in your home folder and set it in the movie <filename> tag with the full path to the movie aka:

/Users/YourUserAccountName/yourMovie.mov

Change the source <useCamera> tag to 0 to use the movie as input. Also check the other movie settings (volume, speed).

GUI controls
------------
The gui has four toggles. "pose" and "gesture" toggle whether to send the pose and gestural stats over OSC, respectively. "raw" toggles whether to send the raw points of the face mesh (66 XY-points = 132 values). "normalize raw" will zero-center and scale the raw points, effectively making the raw points position-invariant (applicable only if "raw" is toggled on).

OSC data
--------

By default, sends to localhost, port 8338. If you want to change this, see the below section about editing the settings.xml file.

Pose
 * center position: /pose/position
 * scale: /pose/scale
 * orientation (which direction you're facing): /pose/orientation

Gestures
 * mouth width: /gesture/mouth/width
 * mouth height: /gesture/mouth/height
 * left eyebrow height: /gesture/eyebrow/left
 * right eyebrow height: /gesture/eyebrow/right
 * left eye openness: /gesture/eye/left
 * right eye openness: /gesture/eye/right
 * jaw openness: /gesture/jaw
 * nostril flate: /gesture/nostrils

Raw
 * raw points (66 xy-pairs): /raw


Key Controls
-------------

* r - reset the face tracker
* m - toggle face mesh drawing
* g - toggle gui visibility
* p - pause/unpause (only works with movie source) 
* up/down - increase/decrease movie playback speed (only orks with movie source)

