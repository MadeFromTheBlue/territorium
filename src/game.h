#ifndef GAME_H
#define GAME_H

#include <string>

#include "resize.h"

struct Loc {
	int x;
	int y;
};

struct Change {
	int x;
	int y;
	int to;
};

int* getBoard();
int getBoardSize();
int getTypeCount();

float getTheta();
float getPhi();

void startLevel(int level);
void resetScore();
void initBoard(int size, int types);

void initBoardAssets();

void preBoardChange();
void sendBoardChange(Change change);
void setChangeDelay(float secs);

void tickBoard(float delta);

void setDarken(Loc at);

void setCameraAngles(float yaw, float pitch);

void drawBlock(int type);
void drawBoard();
void drawUI(float edge, SizedBox& box);

void finishLevel(int totalBlue, int totalRed);
void skipLevel(bool force);

int getScore();
int getGameState();

#endif