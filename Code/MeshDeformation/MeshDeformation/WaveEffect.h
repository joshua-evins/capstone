#pragma once
#include <glm\glm.hpp>

struct WaveEffect
{
	glm::vec3 waveDir;
	float waveDistance;
	glm::vec3 origin;
	bool active;
};