#ifdef __APPLE__			// if compiling on Mac OS
	#include <GLUT/glut.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else						// else compiling on Linux OS
	#include <GL/glut.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include "resize.h"
#include "game.h"

#include <cstdio>
#include <cmath>
#include <stdlib.h>
#include <time.h>

// global variables
#define MENU_SIZE .15
#define EXTRA_SIZE .2

SizedBox box(-EXTRA_SIZE, 1 + EXTRA_SIZE + MENU_SIZE, 0, 1, .5 / (1 + MENU_SIZE), .5, 1); // resize.h
const int frameRate = 30;

double timeCounter = 0;

void changeSize(int w, int h) {
	//save the new window getBoardSize() / height that GLUT informs us about and recac scale
	box.resize(w, h);

	// update the projection matrix based on the window size
	// the GL_PROJECTION matrix governs properties of the view;
	// i.e. what gets seen - use an Orthographic projection that ranges
	// from [0, windowgetBoardSize()] in X and [0, windowHeight] in Y. (0,0) is the lower left.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(box.minX, box.maxX, box.minY, box.maxY, -100, 100);

	// std::printf("%.2f %.2f %.2f %.2f\n", box.minX, box.maxX, box.minY, box.maxY);
	
	// update the viewport as well - tell OpenGL we want to render the whole window
	glViewport(0, 0, box.width, box.height);

	// just as good practice, switch back to GL_MODELVIEW mode; that's the matrix
	// that we want to be changing with glTranslatef(), glScalef(), etc., so
	// just in case we forget to explicitly set it later...
	glMatrixMode(GL_MODELVIEW);
}

void renderScene() {
	// ensure we are drawing to the back buffer
	glDrawBuffer(GL_BACK);

	// clear whatever was drawn to the screen last time - 
	// set the clear color to black and then signal to clear the RGB buffer.
	glClearColor(0, 0, 0, 1);
	glClearDepth(100);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// switch to GL_MODELVIEW, for when we start using glTranslatef(), glScalef(), etc..
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
 
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	drawBoard();
	drawUI(box.maxX - MENU_SIZE, box);

	// flush the OpenGL commands and make sure they get rendered!
	glutSwapBuffers();
}

void frameCallback(int val) {
	tickBoard(1. / frameRate);
	renderScene();
	glutPostRedisplay();
	timeCounter += 1. / frameRate;
	glutTimerFunc(1000 / frameRate, frameCallback, 0);
}

bool lhold = false;
float clickx;
float clicky;

float initialTheta;
float initialPhi;

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		lhold = (state == GLUT_DOWN);
		if (lhold) {
			initialTheta = getTheta();
			initialPhi = getPhi();

			clickx = box.xpoint(x);
			clicky = box.ypoint(y);
		}
	}
}


void mouseMotion(int mx, int my) {
	float x = box.xpoint(mx);
	float y = box.ypoint(my);

	if (lhold && x < (box.maxX - MENU_SIZE)) {
		x -= clickx;
		y -= clicky;

		x /= box.maxX - box.minX;
		y /= box.maxY - box.minY;

		setCameraAngles(x * 360 + initialTheta, y * 90 + initialPhi);
	}
}

void spreadTo(int x, int y, int to) {
	sendBoardChange({x, y, to});
	setDarken({x, y});
}

void standardKeys(unsigned char key, int mx, int my) {	
	if (key > 48 && key <= 48 + getTypeCount()) {
		key -= 47;

		if (getGameState() == 0) {
			preBoardChange();

			int* b = getBoard();
			for (int y = 0; y < getBoardSize(); y++) {
				for (int x = 0; x < getBoardSize(); x++) {
					// for each item on the board
					int i = y * getBoardSize() + x;
					
					// blue
					if (b[i] == 0) {
						if (x > 0 && b[i - 1] == key) spreadTo(x - 1, y, 0);
						if (y > 0 && b[i - getBoardSize()] == key) spreadTo(x, y - 1, 0);
					} else if (b[i] == key) {
						if (x > 0 && b[i - 1] == 0) spreadTo(x, y, 0);
						if (y > 0 && b[i - getBoardSize()] == 0) spreadTo(x, y, 0);
					}

					// red
					if (b[i] == 1) {
						if (x > 0 && b[i - 1] == key) spreadTo(x - 1, y, 1);
						if (y > 0 && b[i - getBoardSize()] == key) spreadTo(x, y - 1, 1);
					} else if (b[i] == key) {
						if (x > 0 && b[i - 1] == 1) spreadTo(x, y, 1);
						if (y > 0 && b[i - getBoardSize()] == 1) spreadTo(x, y, 1);
					}
				}
			}

			setChangeDelay(.5);
		}
	} else if (key == 27) {
		exit(0);
	} else if (key == 'r') {
		startLevel(0);
		resetScore();
	} else if (key == 's') {
		skipLevel(true);
	}
}

//
//  int main(int argc, char* argv[])
//
int main(int argc, char* argv[]) {
	// initialize GLUT... always need this call.
	glutInit(&argc, argv);

	// request a window with just an RGB frame buffer, nothing fancy...
	// place it at (50,50) on the screen and set its size to our constants from earlier
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(50, 10);
	glutInitWindowSize(1080, 720);
	
	// this actually creates our window. 
	// bet you thought the title was going to be a zelda reference, eh? #sworcery
	glutCreateWindow("SQ1: Territorium v1 by Sam Sartor");

	// register our reshape and display callbacks with GLUT - these functions are above
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutMotionFunc(mouseMotion);
	glutMouseFunc(mouse);
	glutKeyboardFunc(standardKeys);

	initBoardAssets();
	srand(time(NULL));
	startLevel(0);

	glutTimerFunc(0, frameCallback, 0);

	// begin GLUT loop
	glutMainLoop();

	return 0;
}
