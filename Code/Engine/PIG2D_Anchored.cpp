#include "PIG2D_Anchored.h"
#include "glm\glm.hpp"
#include "Particle2D.h"

namespace Engine
{
	PIG2D_Anchored::PIG2D_Anchored(glm::vec2* anchor, float springConstant, float restLength)
	{
		this->anchor = anchor;
		this->springConstant = springConstant;
		this->restLength = restLength;
	}


	PIG2D_Anchored::~PIG2D_Anchored()
	{
		delete anchor;
	}

	void PIG2D_Anchored::updateImpulse(Particle2D* particle, float dt)
	{
		glm::vec2 impulse(particle->position);
		impulse -= *anchor;

		float magnitude = (restLength - glm::length(impulse)) * springConstant;

		impulse = glm::normalize(impulse) * magnitude;

		particle->addImpulse(impulse);
	}
}