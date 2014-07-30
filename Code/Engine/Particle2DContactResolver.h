#pragma once
#include "ExportHeader.h"

typedef unsigned int uint;
namespace Engine
{
	class Particle2DContact;
	class ENGINE_SHARED Particle2DContactResolver
	{
	protected:
		uint iterations;
		
	public:
		uint iterationsUsed;
		Particle2DContactResolver(uint iterations);
		void setIterations(uint iterations);
		
		void resolveContacts(Particle2DContact* contactArray,
			uint numContacts,
			float dt);

		~Particle2DContactResolver();
	};
}