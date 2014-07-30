#pragma once
#include <glm\glm.hpp>

class Circle
{
public:
	static float diameters[11];
	static float radii[11];

	Circle* left;
	Circle* right;
	Circle* top;
	Circle* bottom;
	
	float diameter;
	glm::vec2 center;
	Circle(float x, float y, int diameterIndex);
	Circle(float x, float y, float diameter);
};