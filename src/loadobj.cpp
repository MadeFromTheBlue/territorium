/*
* My loader for the Wavefront OBJ format
*/

#include "loadobj.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <utility>
#include <ctype.h>

// OpenGL
#ifdef __APPLE__				// if compiling on Mac OS
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
#else							// if compiling on Linux or Windows OS
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#endif

void loadSpace(const std::string& line, int& at) {
	while (at < line.length() && std::isspace(line[at])) {
		at++;
	}
}

float loadValue(const std::string& line, int& at) {
	std::string num = "";
	while (at < line.length()) {
		char c = line[at];

		if (isspace(c)) break;

		num += c;
		at++;
	}
	return std::stof(num);
}

int loadIndex(const std::string& line, int& at) {
	std::string num = "";
	while (at < line.length()) {
		char c = line[at];

		if (isspace(c)) break;
		if (c == '/') break;

		num += c;
		at++;
	}
	if (num.length() == 0) return 0;
	return std::stoi(num);
}

std::string loadStr(const std::string& line, int& at) {
	std::string str = "";
	while (at < line.length()) {
		char c = line[at];

		if (isspace(c)) break;

		str += c;
		at++;
	}
	return str;
}

void loadVert(const std::string& line, int& at, OBJElems& elems) {
	OBJPos v;
	loadSpace(line, at);
	v.x = loadValue(line, at);
	loadSpace(line, at);
	v.y = loadValue(line, at);
	loadSpace(line, at);
	v.z = loadValue(line, at);
	elems.verts.push_back(v);
}

void loadNorm(const std::string& line, int& at, OBJElems& elems) {
	OBJNorm n;
	loadSpace(line, at);
	n.nx = loadValue(line, at);
	loadSpace(line, at);
	n.ny = loadValue(line, at);
	loadSpace(line, at);
	n.nz = loadValue(line, at);
	elems.norms.push_back(n);
}

void loadTex(const std::string& line, int& at, OBJElems& elems) {
	OBJTex t;
	loadSpace(line, at);
	t.u = loadValue(line, at);
	loadSpace(line, at);
	t.v = loadValue(line, at);
	elems.texs.push_back(t);
}

void loadTriVert(const std::string& line, int& at, OBJTri& tri, int vert, OBJElems& elems) {
	int ind = loadIndex(line, at);
	if (ind != 0) tri.flags |= 1;
	if (ind < 0) ind = elems.verts.size() + ind;
	else ind--;
	tri.pos[vert] = ind;

	if (line[at++] == '/') {
		ind = loadIndex(line, at);
		if (ind != 0) tri.flags |= 2;
		if (ind < 0) ind = elems.texs.size() + ind;
		else ind--;
		tri.tex[vert] = ind;
	}

	if (line[at++] == '/') { 
		ind = loadIndex(line, at);
		if (ind != 0) tri.flags |= 4;
		if (ind < 0) ind = elems.norms.size() + ind;
		else ind--;
		tri.norm[vert] = ind;
	}
}

void loadTri(const std::string& line, int& at, OBJElems& elems) {
	OBJTri tri;
	for (int i = 0; i < 3; i++) {
		loadSpace(line, at);
		loadTriVert(line, at, tri, i, elems);
	}
	elems.tris.push_back(tri);
}

// Loads the data from a Wavefront OBJ file (https://en.wikipedia.org/wiki/Wavefront_.obj_file)
void loadOBJ(std::ifstream& file, OBJElems& elems) {
	std::string line;

	// the name of the current object
	std::string name;
	// the first face num in the current object
	int ostart = -1;

	while (getline(file, line))
	{
		int at = 0;

		loadSpace(line, at);
		std::string op = loadStr(line, at);

		if (op == "v") {
			loadVert(line, at, elems);
		} else if (op == "vn") {
			loadNorm(line, at, elems);
		} else if (op == "vt") {
			loadTex(line, at, elems);
		} else if (op == "f") {
			loadTri(line, at, elems);
		} else if (op == "o") {

			int curTri = (int) elems.tris.size();
			if (ostart >= 0) {
				// end of current object, add object from first to last
				elems.objects.push_back({name, ostart, curTri});
			}

			// start of new current object
			ostart = curTri;

			loadSpace(line, at);
			name = line.substr(at);

		}
	}

	// end of final object
	if (ostart >= 0) {
		elems.objects.push_back({name, ostart, (int) elems.tris.size()});
	}
}

void OBJPos::callGL() const {
	glVertex3f(x, y, z);
}

void OBJNorm::callGL() const {
	glNormal3f(nx, ny, nz);
}

void OBJTex::callGL() const {
	glTexCoord2f(u, v);
}

void OBJTri::callGL(const OBJElems& elems) const {
	for (int i = 0; i < 3; i++) {
		if (flags & 2) elems.texs[tex[i]].callGL();
		if (flags & 4) elems.norms[norm[i]].callGL();
		elems.verts[pos[i]].callGL();
	}
}

// Draw a range of OBJ data 
void drawOBJ(const OBJElems& elems, int fromTri, int toTri) {
	glBegin(GL_TRIANGLES);
	for (int i = fromTri; i < toTri; i++) {
		elems.tris[i].callGL(elems);
	}
	glEnd();
}

// Draw all the data in tris
void drawOBJ(const OBJElems& elems) {
	drawOBJ(elems, 0, elems.tris.size());
}

void drawOBJ(const OBJElems& elems, const OBJObject& object) {
	drawOBJ(elems, object.fromTri, object.toTri);
}