#include "ofMeshUtils.h"

ofVec3f getNormal(const ofVec3f& v1, const ofVec3f& v2, const ofVec3f& v3) {
	ofVec3f a = v1 - v2;
	ofVec3f b = v3 - v2;
	ofVec3f normal = b.cross(a);
	normal.normalize();
	return normal;
}

void buildNormals(ofMesh& mesh) {
	vector<ofVec3f>& vertices = mesh.getVertices();
	for(int i = 0; i < mesh.getNumVertices(); i += 3) {
		ofVec3f normal = getNormal(
															 mesh.getVertices()[i+0],
															 mesh.getVertices()[i+1],
															 mesh.getVertices()[i+2]);
		for(int j = 0; j < 3; j++) {
			mesh.addNormal(normal);
		}
	}
}

ofMesh convertFromIndices(const ofMesh& mesh) {
	ofMesh result;
	// have to do a const_cast because ofMesh::get*() is not const correct
	ofMesh& cmesh = const_cast<ofMesh&>(mesh);
	int vertices = mesh.getNumVertices();
	int colors = mesh.getNumColors();
	int normals = mesh.getNumNormals();
	int texcoords = mesh.getNumTexCoords();
	int indices = mesh.getNumIndices();
	for(int i = 0; i < indices; i++) {
		int cur = cmesh.getIndex(i);
		if(vertices > 0) {
			result.addVertex(cmesh.getVertex(cur));
		}
		if(colors > 0) {
			result.addColor(cmesh.getColor(cur));
		}
		if(normals > 0) {
			result.addNormal(cmesh.getNormal(cur));
		}
		if(texcoords > 0) {
			result.addTexCoord(cmesh.getTexCoord(cur));
		}
	}
	return result;
}