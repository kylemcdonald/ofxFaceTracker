#!/usr/bin/env python

import shutil, glob, os

sourceModelPath = 'libs/FaceTracker/model/'
examples = glob.glob('example*')
for example in examples:
	targetModelPath = example + '/bin/data/'
	try: os.makdirs(targetModelPath)
	except: pass
	try:
		shutil.copytree(sourceModelPath, targetModelPath + 'model/')
		print 'Copied model data into ' + example
	except:
		print 'Did not copy model data into ' + example