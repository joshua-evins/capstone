#pragma once
#include "ExportHeader.h"
#include "Particle2DImpulseGenerator.h"

namespace Engine
{
	class ENGINE_SHARED PIG2D_Spring : public Particle2DImpulseGenerator
	{
		Particle2D* otherParticle;
		float springConstant;
		float restLength;
	public:
		PIG2D_Spring(Particle2D* otherParticle, float springConstant, float restLength);
		~PIG2D_Spring(void);
		virtual void updateImpulse(Particle2D* particle, float dt);
	};
}
