#include "Particle2DContactResolver.h"
#include "MathConstants.h"
#include "Particle2DContact.h"
#include "Particle2D.h"
#include <iostream>

namespace Engine
{
	Particle2DContactResolver::Particle2DContactResolver(uint iterations)
	{
		this->iterations = iterations;
	}

	void Particle2DContactResolver::setIterations(uint iterations)
	{
		this->iterations = iterations;
	}

	Particle2DContactResolver::~Particle2DContactResolver()
	{
	}

	void Particle2DContactResolver::resolveContacts(Particle2DContact* contactArray,
			uint numContacts,
			float dt)
	{
		uint i;
		iterationsUsed = 0;
		
		while(iterationsUsed < iterations)
		{
			float max = FLOAT_MAX;
			uint maxIndex = numContacts;

			for(i=0; i<numContacts; i++)
			{
				float sepVel = contactArray[i].calculateSeparatingVelocity();
				if(sepVel < max &&
					(sepVel < 0 || contactArray[i].penetration > 0))
				{
					max = sepVel;
					maxIndex = i;
				}
			}

			if(maxIndex == numContacts)
				break;

			contactArray[maxIndex].resolve(dt);

			iterationsUsed++;
		}
	}
}