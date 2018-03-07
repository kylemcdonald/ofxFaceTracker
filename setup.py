#!/usr/bin/env python

import glob, re, shutil, fileinput, os

sourceProjectName = "EmptyExample"
sourceModelPath = "libs/FaceTracker/model/"

examples = glob.glob("example*")
for example in examples:
    # model data
    targetModelPath = example + "/bin/data/model/"
    try: shutil.rmtree(targetModelPath)
    except: pass
    shutil.copytree(sourceModelPath, targetModelPath)

    print ("Copied model data into " + targetModelPath)

# Don't forget FACEOSC.
targetModelPath = "FaceOSC/bin/data/model/"
try: shutil.rmtree(targetModelPath)
except: pass
shutil.copytree(sourceModelPath, targetModelPath)

print ("Copied model data into " + targetModelPath)
