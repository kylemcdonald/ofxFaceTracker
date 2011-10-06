#pragma once

#include "ofMesh.h"

ofVec3f getNormal(const ofVec3f& v1, const ofVec3f& v2, const ofVec3f& v3);
void buildNormals(ofMesh& mesh);
ofMesh convertFromIndices(const ofMesh& mesh);