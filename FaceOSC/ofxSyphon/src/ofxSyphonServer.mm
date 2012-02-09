/*
 *  ofxSyphonServer.cpp
 *  syphonTest
 *
 *  Created by astellato,vade,bangnoise on 11/6/10.
 *  
 *  http://syphon.v002.info/license.php
 */

#include "ofxSyphonServer.h"
#import <Syphon/Syphon.h>

ofxSyphonServer::ofxSyphonServer()
{
	mSyphon = nil;
}

ofxSyphonServer::~ofxSyphonServer()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    [(SyphonServer *)mSyphon stop];
    [(SyphonServer *)mSyphon release];
    
    [pool drain];
}


void ofxSyphonServer::setName(string n)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
	NSString *title = [NSString stringWithCString:n.c_str() 
										 encoding:[NSString defaultCStringEncoding]];
	
	if (!mSyphon)
	{
		mSyphon = [[SyphonServer alloc] initWithName:title context:CGLGetCurrentContext() options:nil];
	}
	else
	{
		[(SyphonServer *)mSyphon setName:title];
	}
    
    [pool drain];
}

string ofxSyphonServer::getName()
{
	string name;
	if (mSyphon)
	{
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

		name = [[(SyphonServer *)mSyphon name] cStringUsingEncoding:[NSString defaultCStringEncoding]];
		
		[pool drain];
	}
	else
	{
		name = "Untitled";
	}
	return name;
}

void ofxSyphonServer::publishScreen()
{
	int w = ofGetWidth();
	int h = ofGetHeight();
	
	ofTexture tex;
	tex.allocate(w, h, GL_RGBA);
	
	tex.loadScreenData(0, 0, w, h);
			
	this->publishTexture(&tex);
	
	tex.clear();
 }


void ofxSyphonServer::publishTexture(ofTexture* inputTexture)
{
    // If we are setup, and our input texture
	if(inputTexture->bAllocated())
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
		ofTextureData texData = inputTexture->getTextureData();

		if (!mSyphon)
		{
			mSyphon = [[SyphonServer alloc] initWithName:@"Untitled" context:CGLGetCurrentContext() options:nil];
		}
		
		[(SyphonServer *)mSyphon publishFrameTexture:texData.textureID textureTarget:texData.textureTarget imageRegion:NSMakeRect(0, 0, texData.width, texData.height) textureDimensions:NSMakeSize(texData.width, texData.height) flipped:!texData.bFlipTexture];
        [pool drain];
    } 
    else 
    {
		cout<<"ofxSyphonServer texture is not properly backed.  Cannot draw.\n";
	}
}

