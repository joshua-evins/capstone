#pragma once
#include "ExportHeader.h"
#include "Particle2DImpulseGenerator.h"

namespace Engine
{
	class Particle2D;
	class ENGINE_SHARED PIG2D_Drag : public Particle2DImpulseGenerator
	{
	public:
		float k1;
		float k2;
		PIG2D_Drag(float k1, float k2);
		virtual void updateImpulse(Particle2D* particle, float dt);
	};
}
