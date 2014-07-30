#pragma once
#include "ExportHeader.h"
#include "glm\glm.hpp"
#include "Particle2DImpulseGenerator.h"

namespace Engine
{
	class Particle2D;

	class ENGINE_SHARED PIG2D_Anchored : public Particle2DImpulseGenerator
	{
		glm::vec2* anchor;

		float springConstant;
		float restLength;
	public:
		PIG2D_Anchored(glm::vec2* anchor, float springConstant, float restLength);
		~PIG2D_Anchored();
		virtual void updateImpulse(Particle2D* particle, float dt);
	};
}