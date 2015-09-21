#!/usr/bin/env python

import shutil, glob, os, errno

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

sourceModelPath = 'libs/FaceTracker/model/'
examples = glob.glob('example*')
for example in examples:
	targetModelPath = example + '/bin/data/'
	mkdir_p(targetModelPath)
	try:
		shutil.copytree(sourceModelPath, targetModelPath + 'model/')
		print 'Copied model data into ' + example
	except:
		print 'Did not copy model data into ' + example