#include "PIG2D_Spring.h"
#include "glm\glm.hpp"
#include "Particle2D.h"

namespace Engine
{
	PIG2D_Spring::PIG2D_Spring(Particle2D* otherParticle, float springConstant, float restLength)
	{
		this->otherParticle = otherParticle;
		this->springConstant = springConstant;
		this->restLength = restLength;
	}


	PIG2D_Spring::~PIG2D_Spring(void)
	{
	}

	void PIG2D_Spring::updateImpulse(Particle2D* particle, float dt)
	{
		glm::vec2 impulse(particle->position);
		impulse -= otherParticle->position;

		float magnitude = glm::length(impulse);
		
		magnitude -= restLength;
		magnitude = (magnitude < 0) ? -magnitude : magnitude;
		
		magnitude *= springConstant;

		impulse = glm::normalize(impulse);
		impulse *= -magnitude;
		particle->addImpulse(impulse);
	}
}