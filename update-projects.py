#!/bin/python

import glob, re, shutil, fileinput, os

def toUpper(pattern):
  return pattern.group(1).upper()
	
def replaceInFile(filename, pattern, replacement):
  for line in fileinput.FileInput(filename, inplace=1):
    print re.sub(pattern, replacement, line),

sourceProjectName = "EmptyExample"
sourceModelPath = "libs/FaceTracker/model/"

# windows code::blocks
sourceCbp = "example-empty/EmptyExample.cbp"
sourceWorkspace = "example-empty/EmptyExample.workspace"

# xcode osx
sourceXcconfig = "example-empty/Project.xcconfig"
sourcePlist = "example-empty/openFrameworks-Info.plist"
sourceXcodeproj = "example-empty/ofApp.xcodeproj/"

examples = glob.glob("example*")
for example in examples:
  if example != "example-empty":
    sansExample = re.sub("^example", "", example)
    upper = re.sub("-([a-z])", toUpper, sansExample)
    targetProjectName = upper + "Example"

    # windows code::blocks
    targetCbp = "{0}/{1}.cbp".format(example, targetProjectName)
    targetWorkspace = "{0}/{1}.workspace".format(example, targetProjectName)
    shutil.copy(sourceCbp, targetCbp)
    shutil.copy(sourceWorkspace, targetWorkspace)
    replaceInFile(targetCbp, sourceProjectName, targetProjectName)
    replaceInFile(targetWorkspace, sourceProjectName, targetProjectName)

    # xcode osx
    targetXcodeproj = "{0}/ofApp.xcodeproj".format(example)
    shutil.copy(sourceXcconfig, example)
    shutil.copy(sourcePlist, example)
    if not os.path.exists(targetXcodeproj):
      os.mkdir(targetXcodeproj)
    try: shutil.rmtree(targetXcodeproj)
    except: pass
    shutil.copytree(sourceXcodeproj, targetXcodeproj)

    print "Updated " + targetProjectName + " project."

  # model data
  targetModelPath = example + "/bin/data/model/"
  try: shutil.rmtree(targetModelPath)
  except: pass
  shutil.copytree(sourceModelPath, targetModelPath)

  print "Copied model data into " + targetModelPath