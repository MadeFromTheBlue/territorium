/*
* My loader for the Wavefront OBJ format
*/

#ifndef LOADOBJ_H
#define LOADOBJ_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

struct OBJPos {
	float x;
	float y;
	float z;

	void callGL() const;
};

struct OBJNorm {
	float nx;
	float ny;
	float nz;

	void callGL() const;
};

struct OBJTex {
	float u;
	float v;

	void callGL() const;
};

struct OBJTri;

struct OBJObject {
	std::string name;
	int fromTri;
	int toTri;
};

struct OBJElems {
	std::vector<OBJPos> verts;
	std::vector<OBJNorm> norms;
	std::vector<OBJTex> texs;
	std::vector<OBJTri> tris;
	std::vector<OBJObject> objects;
};


struct OBJTri {
	char flags = 0;
	int pos[3] = {-1, -1, -1};
	int tex[3] = {-1, -1, -1};
	int norm[3] = {-1, -1, -1};

	void callGL(const OBJElems& elems) const;
};


void loadOBJ(std::ifstream& file, OBJElems& elems);

void drawOBJ(const OBJElems& elems, const OBJObject& object);

void drawOBJ(const OBJElems& elems, int fromTri, int toTri);

void drawOBJ(const OBJElems& elems);

#endif