#ifndef SIZEDBOX_H
#define SIZEDBOX_H

class SizedBox {
private:
	float askedMinX;
	float askedMinY;
	float askedMaxX;
	float askedMaxY;

	float alignX;
	float alignY;

	float fit;

public:
	int width;
	int height;

	float minX;
	float minY;
	float maxX;
	float maxY;
	float sizeX;
	float sizeY;

	SizedBox(float minX, float maxX, float minY, float maxY, float alignX, float alignY, float fit):
	askedMinX(minX),
	askedMinY(minY),
	askedMaxX(maxX),
	askedMaxY(maxY),
	alignX(alignX),
	alignY(alignY),
	fit(fit) {};

	void resize(int w, int h);

	inline float xpoint(int preX) {
		return minX + (preX / (float) width) * sizeX;
	};

	inline float ypoint(int preY) {
		return maxY - (preY / (float) height) * sizeY;
	};

	inline int screenx(float preX) {
		return (int) (width * (preX - minX) / (maxX - minX));
	};

	inline int screeny(float preY) {
		return (int) (height * (preY - minY) / (maxY - minY));
	};
};

#endif