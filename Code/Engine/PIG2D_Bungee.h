#pragma once
#include "ExportHeader.h"
#include "glm\glm.hpp"
#include "Particle2DImpulseGenerator.h"

namespace Engine
{
	class Particle2D;

	class ENGINE_SHARED PIG2D_Bungee : public Particle2DImpulseGenerator
	{
		Particle2D* otherParticle;

		float springConstant;
		float restLength;
	public:
		PIG2D_Bungee(Particle2D* otherParticle, float springConstant, float restLength);
		~PIG2D_Bungee();
		virtual void updateImpulse(Particle2D* particle, float dt);
	};
}