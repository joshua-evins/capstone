#pragma once
#include <glm\glm.hpp>

struct RippleEffect
{
	glm::vec4 origin;
	float rippleDistance;
	int active;
	float paddingf[2]; // I am not amused
};