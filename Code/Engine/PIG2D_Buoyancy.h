#pragma once
#include "ExportHeader.h"
#include "Particle2DImpulseGenerator.h"

namespace Engine
{
	class ENGINE_SHARED PIG2D_Buoyancy : public Particle2DImpulseGenerator
	{
		float maxDepth;
		float volume;
		float waterHeight;
		float liquidDensity;
	public:
		PIG2D_Buoyancy(float maxDepth, float volume, float waterHeight, float liquidDensity = 1000.0f);
		~PIG2D_Buoyancy(void);
		virtual void updateImpulse(Particle2D* particle, float dt);
	};
}