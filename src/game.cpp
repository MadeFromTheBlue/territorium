#ifdef __APPLE__			// if compiling on Mac OS
	#include <GLUT/glut.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else						// else compiling on Linux OS
	#include <GL/glut.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#ifdef __linux
	#include <SOIL/SOIL.h>
#else
	#include <SOIL/soil.h>
#endif

#include "game.h"
#include "loadobj.h"

#include <vector>
#include <set>
#include <utility>
#include <stdlib.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>

int boardSize;
int blockTypes;
int* board;
float* offset;

OBJElems models;
OBJObject blockModels[100];
OBJObject digits[10];

OBJObject victory;
OBJObject defeat;
OBJObject press;

bool operator<(const Loc& a, const Loc& b) {
	if (a.x < b.x) return true;
	if (a.x > b.x) return false;
	return a.y < b.y;
}

std::set<Loc> darken;
std::vector<Change> changes;

bool inAni = false;
float turnAniLength = 0;
float turnAni = 0;
float secs = 0;

float theta = 45;
float phi = 54.736;

float drawDiameter;

int currentLevel;
bool isTutorial;
int score;
int state;

int getScore() { return score; }

int getGameState() { return state; }

float getTheta() { return theta; }

float getPhi() { return phi; }

int getTypeCount() { return blockTypes; }

void initBoardAssets() {
	std::ifstream file("models.obj");
	loadOBJ(file, models);
	file.close();

	for (OBJObject o : models.objects) {
		std::string name = o.name;
		int delim = (int) name.find(':');
		std::string type = name.substr(0, delim);
		std::string sub = name.substr(delim + 1);

		if (type == "block") {
			blockModels[std::stoi(sub)] = o;
		} else if (type == "deg") {
			digits[std::stoi(sub)] = o;
		} else if (type == "text") {
			if (sub == "victory") victory = o;
			else if (sub == "defeat") defeat = o;
			else if (sub == "press") press = o;
		}
	}

	glEnable(GL_TEXTURE_2D);

	GLint mapTexHandle = SOIL_load_OGL_texture("./models.png",
											SOIL_LOAD_AUTO,
											SOIL_CREATE_NEW_ID,
											SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y| SOIL_FLAG_COMPRESS_TO_DXT);

	glBindTexture(GL_TEXTURE_2D, mapTexHandle);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int randomBlock() {
	return (rand() % blockTypes) + 2;
}

void loadLevelFileTile(int x, int y, char c, int* permute) {
	int i = y * boardSize + x;

	if (c >= 48 && c <= 58) {
		board[i] = permute[c - 48] + 2;
	} else if (c == 'R') {
		board[i] = randomBlock();
	} else if (c == '.') {
		board[i] = 0;
	} else if (c == '!') {
		board[i] = 1;
	} else {
		board[i] = -1;
	}

	offset[i] = 0;
}

void loadLevelFileRow(int row, const std::string line, int* permute) {
	for (int i = 0; i < boardSize; i++) {
		loadLevelFileTile(boardSize - i - 1, boardSize - row - 1, line[i], permute);
	}
}

int* randPermute(int size, bool shuff) {
	int* perm = new int[size];
	for (int i = 0; i < size; i++) {
		perm[i] = i;
	}

	if (!shuff) return perm;

	for (int i = size - 1; i >= 0; i--){
		int j = rand() % (i + 1);

		int temp = perm[i];
		perm[i] = perm[j];
		perm[j] = temp;
	}

	return perm;
}

bool loadLevelFile(int level) {
	std::ostringstream fname;
	fname << "levels/" << level << ".txt";

	std::ifstream file(fname.str());
	if (!file.is_open()) return false;

	printf("Loading \"%s\"...\n", fname.str().c_str());

	int size;
	int count;

	int* permute = NULL;

	std::string line;
	int linenum = 0;
	while (getline(file, line))
	{
		if (line[0] == '#') continue;

		switch (linenum) {
			case 0:
				size = std::stoi(line);
				break;
			case 1:
				count = std::stoi(line);
				break;
			case 2:
				initBoard(size, count);
				drawDiameter = std::stof(line);
				break;
			case 3:
				isTutorial = (line[0] == 't');
				break;
			case 4:
				permute = randPermute(count, line[0] == 't');
		}

		if (linenum >= 5) {
			loadLevelFileRow(linenum - 5, line, permute);
		}

		linenum++;
	}

	delete permute;

	file.close();
	return true;
}

void startLevel(int level) {
	state = 0;
	currentLevel = level;

	if (!loadLevelFile(level)) {
		printf("Could not load \"levels/%d.txt\". Defaulting to random 7x7.\n", level);

		initBoard(7, 3);
		int ssq = boardSize * boardSize;
		for (int i = 0; i < ssq; i++) {
			board[i] = randomBlock();
		}
		board[0] = 0;
		board[ssq - 1] = 1;
		drawDiameter = boardSize;
	}
}

void skipLevel(bool force) {
	if (force || isTutorial) {
		startLevel(currentLevel + 1);
	}
}

void resetScore() {
	score = 0;
}

void initBoard(int size, int count) {
	blockTypes = count;
	boardSize = size;

	int sizesq = size * size;

	if (board != NULL) delete[] board;
	board = new int[sizesq];

	if (offset != NULL) delete[] offset;
	offset = new float[sizesq];
}

int* getBoard() { return board; }

int getBoardSize() { return boardSize; }

void sendBoardChange(Change change) {
	if (change.to != board[change.x + change.y * boardSize]) {
		changes.push_back(change);
	}
}

void endChangeDelay() {
	if (state == 0) {
		darken.clear();
		for (Change c : changes) {
			int i = c.y * boardSize + c.x;
			board[i] = c.to;
			if (c.to < 2) offset[i] = secs;
		}
		changes.clear();
	} else if (state == 1) {
		startLevel(currentLevel + 1);
	}
}

void setChangeDelay(float secs) {
	inAni = true;
	turnAniLength = secs;
	turnAni = 0;
}

void preBoardChange() {
	if (inAni) endChangeDelay();
}

void tickBoard(float delta) {
	if (inAni) {
		turnAni += delta / turnAniLength;
		if (turnAni > 1) {
			inAni = false;
			endChangeDelay();
		}
	}
	secs += delta;

	if (state == 0) {
		int totalBlue = 0;
		int totalRed = 0;
		int total = 0;

		for (int i = 0; i < boardSize * boardSize; i++) {
			if (board[i] == 0) totalBlue++;
			else if (board[i] == 1) totalRed++;

			if (board[i] != -1) total++;
		}

		if (totalBlue + totalRed >= total) {
			finishLevel(totalBlue, totalRed);
		}
	}
}

void setDarken(Loc at) {
	darken.insert(at);
}

void drawBlock(int type) {
	drawOBJ(models, blockModels[type]);
}

void drawBoard() {
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();

	float scale = 1. / drawDiameter;
	glScalef(scale, scale, scale);

	float half = drawDiameter / 2.;
	glTranslatef(0, half, 0);
	glRotatef(-phi, 1, 0, 0);

	glTranslatef(half, 0, 0);
	glRotatef(theta, 0, 0, 1);
	glTranslatef(-half, -half, 0);

	for (int x = 0; x < boardSize; x++) {
		for (int y = 0; y < boardSize; y++) {
			int i = y * boardSize + x;
			int type = board[i];

			if (type < 0) continue;

			glPushMatrix();

			glTranslatef(x + .5, y + .5, 0);

			if (inAni && darken.count({x, y})) {
				float val = 1 - turnAni;
				glColor3f(val, val, val);
			} else {
				glColor3f(1, 1, 1);
			}

			glTranslatef(.5, .5, -1);
			if (type < 2) glTranslatef(0, 0, .1 * sin((secs - offset[i]) * 1.2));
			drawBlock(type);

			glPopMatrix();
		}
	}

	glPopMatrix();
}

void setCameraAngles(float yaw, float pitch) {
	theta = yaw;
	phi = pitch;
}

void drawDigit(int dig) {
	drawOBJ(models, digits[dig]);
}

void drawNumber(int n) {
	int f = n;
	int i = 0;
	while (true) {
		i++;
		f /= 10;
		if (f == 0) break;
	}

	const float w = .8;

	glTranslatef(-w * i, 0, 0);
	while (true) {
		drawDigit(n % 10);
		glTranslatef(w, 0, 0);
		n /= 10;
		if (n == 0) break;
	}
}

void drawUIText(SizedBox& box, bool color) {
	glPushMatrix();
	if (state == 0) {
		if (color) glColor3f(.6, .6, .6);
		glTranslatef(.5, box.maxY - .01, 0);
		glScalef(-.2, -.2, .2);
		drawOBJ(models, press);
	} else {
		if (color) {
			if (state == 1) glColor3f(.2, .2, .8);
			else glColor3f(.8, .2, .2);
		}
		glTranslatef(.5, .5, 0);
		glScalef(-.2, -.2, .2);
		drawOBJ(models, (state == 1) ? victory : defeat);

		glTranslatef(.3, .5, 0);
		glScalef(.3, .3, .3);
		drawNumber(score);
	}
	glPopMatrix();
}

void drawUI(float edge, SizedBox& box) {
	glPushMatrix();

	float menuSize = box.maxX - edge;

	glTranslatef(0, 0, 10);

	glDisable(GL_TEXTURE_2D);

	glColor3f(.6, .6, .6);
	glBegin(GL_QUADS);
	glVertex2f(edge - .01, box.minY);
	glVertex2f(edge - .01, box.maxY);
	glVertex2f(box.maxX, box.maxY);
	glVertex2f(box.maxX, box.minY);
	glEnd();

	glTranslatef(0, 0, 1);

	glColor3f(.8, .8, .8);
	glBegin(GL_QUADS);
	glVertex2f(edge, box.minY);
	glVertex2f(edge, box.maxY);
	glVertex2f(box.maxX, box.maxY);
	glVertex2f(box.maxX, box.minY);
	glEnd();

	glTranslatef(0, 0, 4);

	glLineWidth(3);
	glColor3f(0, 0, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawUIText(box, false);

	glTranslatef(0, 0, 1);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	drawUIText(box, true);

	float scale = menuSize * .6;

	float xpos = edge;
	float ypos = box.maxY - scale * .1;

	for (int i = 0; i < blockTypes; i++) {
		glPushMatrix();

		glTranslatef(xpos + scale * .1, ypos, 0);

		glPushMatrix();
		glTranslatef(0, -scale / 2, 0);
		glScalef(-scale * .3, -scale * .3, scale * .3);
		glTranslatef(0, .5, 0);
		glColor3f(.2, .2, .2);
		drawDigit(i + 1);
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();

		glTranslatef(scale * .8, 0, 0);
		glScalef(scale, scale, scale);
		glTranslatef(.5, 0, 0);
		glColor3f(1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		drawBlock(i + 2);

		glPopMatrix();

		ypos -= 1.2 * scale;
	}

	glPopMatrix();
}

void finishLevel(int totalBlue, int totalRed) {
	score += totalBlue;
	printf("B: %d vs R: %d ... Score: %d\n", totalBlue, totalRed, score);
	if (totalBlue > totalRed) {
		state = 1;
		setChangeDelay(2);
	} else {
		state = 2;
	}
}