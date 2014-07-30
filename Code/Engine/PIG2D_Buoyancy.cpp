#include "PIG2D_Buoyancy.h"
#include "Particle2D.h"
#include <iostream>

namespace Engine
{
	PIG2D_Buoyancy::PIG2D_Buoyancy(float maxDepth, float volume, float waterHeight, float liquidDensity)
	{
		this->maxDepth = maxDepth;
		this->volume = volume;
		this->waterHeight = waterHeight;
		this->liquidDensity = liquidDensity;
	}

	PIG2D_Buoyancy::~PIG2D_Buoyancy()
	{
	}

	void PIG2D_Buoyancy::updateImpulse(Particle2D* particle, float dt)
	{
		float height = particle->position.y;

		if (height >= waterHeight + maxDepth) return;

		glm::vec2 impulse(0,0);
		
		if (height <= waterHeight - maxDepth)
		{
		impulse.y = liquidDensity * volume;
		particle->addImpulse(impulse);
		return;
		}

		float amountAboveSurface = height - waterHeight;
		float percentSubmerged = amountAboveSurface / maxDepth;

		impulse.y = liquidDensity * volume * percentSubmerged;
		//impulse.y = 1;
		particle->addImpulse(impulse);
	}
}