#pragma once
#include "ExportHeader.h"

namespace Engine
{
	class Particle2D;

	class ENGINE_SHARED Particle2DImpulseGenerator
	{
	public:
		virtual void updateImpulse(Particle2D* particle, float dt) = 0;
	};
}
