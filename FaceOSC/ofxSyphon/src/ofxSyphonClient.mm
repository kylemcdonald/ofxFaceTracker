/*
 *  ofxSyphonServer.cpp
 *  syphonTest
 *
 *  Created by astellato,vade,bangnoise on 11/6/10.
 *  
 *  http://syphon.v002.info/license.php
 */

#include "ofxSyphonClient.h"
#import <Syphon/Syphon.h>
#import "SyphonNameboundClient.h"

ofxSyphonClient::ofxSyphonClient()
{
	bSetup = false;
}

ofxSyphonClient::~ofxSyphonClient()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    [(SyphonNameboundClient*)mClient release];
    mClient = nil;
    
    [pool drain];
}

void ofxSyphonClient::setup()
{
    // Need pool
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
	mClient = [[SyphonNameboundClient alloc] init]; 
               
	bSetup = true;
    
    [pool drain];
}

void ofxSyphonClient::setApplicationName(string appName)
{
    if(bSetup)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSString *name = [NSString stringWithCString:appName.c_str() encoding:[NSString defaultCStringEncoding]];
        
        [(SyphonNameboundClient*)mClient setAppName:name];

        [pool drain];
    }
    
}
void ofxSyphonClient::setServerName(string serverName)
{
    if(bSetup)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSString *name = [NSString stringWithCString:serverName.c_str() encoding:[NSString defaultCStringEncoding]];

        if([name length] == 0)
            name = nil;
        
        [(SyphonNameboundClient*)mClient setName:name];
    
        [pool drain];
    }    
}

void ofxSyphonClient::bind()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    if(bSetup)
    {
     	[(SyphonNameboundClient*)mClient lockClient];
        SyphonClient *client = [(SyphonNameboundClient*)mClient client];
   
        latestImage = [client newFrameImageForContext:CGLGetCurrentContext()];
		NSSize texSize = [(SyphonImage*)latestImage textureSize];
                
        // we now have to manually make our ofTexture's ofTextureData a proxy to our SyphonImage
        mTex.texData.textureID = [(SyphonImage*)latestImage textureName];
        mTex.texData.textureTarget = GL_TEXTURE_RECTANGLE_ARB;  // Syphon always outputs rect textures.
        mTex.texData.width = texSize.width;
        mTex.texData.height = texSize.height;
        mTex.texData.tex_w = texSize.width;
        mTex.texData.tex_h = texSize.height;
        mTex.texData.tex_t = texSize.width;
        mTex.texData.tex_u = texSize.height;
        mTex.texData.glType = GL_RGBA;
        mTex.texData.pixelType = GL_UNSIGNED_BYTE;
        mTex.texData.bFlipTexture = YES;
        mTex.texData.bAllocated = YES;
        
        mTex.bind();
    }
    else
		cout<<"ofxSyphonClient is not setup, or is not properly connected to server.  Cannot bind.\n";
    
    [pool drain];
}

void ofxSyphonClient::unbind()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    if(bSetup)
    {
        mTex.unbind();

        [(SyphonNameboundClient*)mClient unlockClient];
        [(SyphonImage*)latestImage release];
        latestImage = nil;
    }
    else
		cout<<"ofxSyphonClient is not setup, or is not properly connected to server.  Cannot unbind.\n";

        [pool drain];
}

void ofxSyphonClient::draw(float x, float y, float w, float h)
{
    this->bind();
    
    mTex.draw(x, y, w, h);
    
    this->unbind();
}

void ofxSyphonClient::draw(float x, float y)
{
	this->draw(x,y, mTex.texData.width, mTex.texData.height);
}







