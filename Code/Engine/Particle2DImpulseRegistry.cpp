#include "Particle2DImpulseRegistry.h"
#include "Particle2DImpulseGenerator.h"

namespace Engine
{
	Particle2DImpulseRegistry::ParticleImpulseRegistration::ParticleImpulseRegistration(Particle2D* particle, Particle2DImpulseGenerator* fg)
	{
		this->particle = particle;
		this->fg = fg;
	}

	void Particle2DImpulseRegistry::add(Particle2D* particle, Particle2DImpulseGenerator* fg)
	{
		registrations.push_back(ParticleImpulseRegistration(particle, fg));
	}

	void Particle2DImpulseRegistry::remove(Particle2D* particle, Particle2DImpulseGenerator* fg)
	{
		Registry::iterator it = registrations.begin();
		for(unsigned int i=0; i<registrations.size(); i++)
		{
			if((it+i)->particle == particle && (it+i)->fg == fg)
			{
				registrations.erase(it+i);
			}
		}
	}

	void Particle2DImpulseRegistry::clear()
	{
		registrations.clear();
	}

	int Particle2DImpulseRegistry::size()
	{
		return registrations.size();
	}

	void Particle2DImpulseRegistry::updateForces(float dt)	
	{
		Registry::iterator i = registrations.begin();
		for(; i != registrations.end(); i++)
			i->fg->updateImpulse(i->particle, dt);
	}
}