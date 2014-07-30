#pragma once
#include <glm\glm.hpp>
#include "ExportHeader.h"

namespace Engine
{
	class Particle2D;
	class ENGINE_SHARED Particle2DContact
	{
	public:

		Particle2D* particle[2];

		float restitution;
		float penetration;
		
		glm::vec2 contactNormal;
		glm::vec2 particleMovement[2];

		void resolve(float dt);

		float calculateSeparatingVelocity() const;

	private:
		void resolveVelocity(float dt);
		void resolveInterpenetration(float dt);
	};
}