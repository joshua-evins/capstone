#include "PIG2D_Bungee.h"
#include "Particle2D.h"

namespace Engine
{
	PIG2D_Bungee::PIG2D_Bungee(Particle2D* otherParticle, float springConstant, float restLength)
	{
		this->otherParticle = otherParticle;
		this->springConstant = springConstant;
		this->restLength = restLength;
	}


	PIG2D_Bungee::~PIG2D_Bungee()
	{
	}

	void PIG2D_Bungee::updateImpulse(Particle2D* particle, float dt)
	{
		glm::vec2 impulse(particle->position);
		impulse -= otherParticle->position;

		float magnitude = glm::length(impulse);

		if(magnitude <= restLength) return;

		magnitude -= restLength;
		magnitude = (magnitude < 0) ? -magnitude : magnitude;
		
		magnitude *= springConstant;

		impulse = glm::normalize(impulse);
		impulse *= -magnitude;
		particle->addImpulse(impulse);
	}
}