#include "resize.h"

void SizedBox::resize(int w, int h) {
	width = w;
	height = h;

	// fit/fill in window and fix stretching
	float scaleRatioX = (askedMaxX - askedMinX) / (float) width;
	float scaleRatioY = (askedMaxY - askedMinY) / (float) height;

	float scaleRatio;
	if (scaleRatioX > scaleRatioY) {
		scaleRatio = scaleRatioX * fit + scaleRatioY * (1 - fit);
	} else {
		scaleRatio = scaleRatioY * fit + scaleRatioX * (1 - fit);
	}

	float centerX = askedMinX * (1. - alignX) + askedMaxX * alignX;
	float centerY = askedMinY * (1. - alignY) + askedMaxY * alignY;

	sizeX = scaleRatio * width;
	sizeY = scaleRatio * height;

	minX = centerX - sizeX * alignX;
	maxX = centerX + sizeX * (1. - alignX);
	minY = centerY - sizeY * alignY;
	maxY = centerY + sizeY * (1. - alignY);
}