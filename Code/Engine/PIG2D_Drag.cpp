#include "PIG2D_Drag.h"
#include "Particle2D.h"

namespace Engine
{
	PIG2D_Drag::PIG2D_Drag(float k1, float k2)
	{
		this->k1 = k1;
		this->k2 = k2;
	}

	void PIG2D_Drag::updateImpulse(Particle2D* particle, float dt)
	{
		glm::vec2 impulse(particle->velocity);
		
		// Calculate the total drag coefficient.
		float speed = glm::length(impulse);
		float dragCoeff = k1 * speed + k2 * speed * speed;
		// Calculate the final force and apply it.
		impulse = glm::normalize(impulse);
		impulse *= -dragCoeff;
		particle->addImpulse(impulse);
	}
}