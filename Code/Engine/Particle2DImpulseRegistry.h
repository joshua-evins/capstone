#pragma once
#include <vector>
#include "ExportHeader.h"

namespace Engine
{
	class Particle2D;
	class Particle2DImpulseGenerator;

	class ENGINE_SHARED Particle2DImpulseRegistry
	{
	protected:
		struct ParticleImpulseRegistration
		{
			Particle2D* particle;
			Particle2DImpulseGenerator* fg;
			ParticleImpulseRegistration(Particle2D* particle, Particle2DImpulseGenerator* fg);
		};
		typedef std::vector<ParticleImpulseRegistration> Registry;
		Registry registrations;
		
	public:
		void add(Particle2D* particle, Particle2DImpulseGenerator* fg);
		void remove(Particle2D* particle, Particle2DImpulseGenerator* fg);
		void clear();
		void updateForces(float dt);
		int size();
	};
}