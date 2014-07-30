#include "Particle2D.h"
#include "MathConstants.h"

namespace Engine
{
	#pragma region Constructors and Destructors
		Particle2D::Particle2D() : position(0.0f, 0.0f), velocity(0.0f, 0.0f), acceleration(0.0f, 0.0f)
		{
			active = false;
			inverseMass = 1.0f;
			damping = 1.0f;
		}

		Particle2D::~Particle2D()
		{}
	#pragma endregion

	#pragma region inverseMass Accessors / Mutators
		float Particle2D::getInverseMass()
		{
			return inverseMass;
		}

		void Particle2D::setInverseMass(float inverseMass)
		{
			this->inverseMass = inverseMass;
		}

		float Particle2D::getMass()
		{
			return (inverseMass == 0) ? FLOAT_MAX : 1/inverseMass;
		}

		void Particle2D::setMass(float mass)
		{
			inverseMass = (mass == 0) ? FLOAT_MAX : 1/mass;
		}

	#pragma endregion

		float Particle2D::getMassRadius()
		{
			float massRadius = getMass()+2;
			return (massRadius > 15) ? 15 : massRadius;
		}

	#pragma region Impulse and Force
		void Particle2D::addImpulse(glm::vec2 impulse)
		{
			impulseSum += impulse;
		}

		void Particle2D::applyImpulseSum(float dt)
		{
			 velocity += impulseSum * inverseMass * dt;
		}

		void Particle2D::clearImpulseSum()
		{
			impulseSum = glm::vec2(0,0);
		}

		void Particle2D::applyForce(glm::vec2 force)
		{
			acceleration += force * inverseMass;
		}
	#pragma endregion
	
	void Particle2D::integrate(float dt)
	{
		if (inverseMass <= 0.0f)
            return;

        //assert(dt > 0.0f);
		
        position += velocity * dt;

		applyImpulseSum(dt);
		
		velocity *= damping;
        velocity += acceleration * dt;

		clearImpulseSum();
	}
}