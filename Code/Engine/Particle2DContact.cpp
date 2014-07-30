#include "Particle2DContact.h"
#include "Particle2D.h"
#include "typedefs_Engine.h"
#include <iostream>

namespace Engine
{
	void Particle2DContact::resolve(float dt)
	{
		resolveVelocity(dt);
		resolveInterpenetration(dt);
	}

	float Particle2DContact::calculateSeparatingVelocity() const
	{
		glm::vec2 relativeVelocity(particle[0]->velocity);
		if(particle[1])
			relativeVelocity -= particle[1]->velocity;
		
		return glm::dot(relativeVelocity, contactNormal);
	}

	void Particle2DContact::resolveVelocity(float dt)
	{
		float separatingVelocity = calculateSeparatingVelocity();

		if(separatingVelocity > 0)
			return;

		particle[0]->velocity -= particle[0]->acceleration * dt;
		if(particle[1])
		{
			particle[1]->velocity -= particle[1]->acceleration * dt;
		}
		separatingVelocity = calculateSeparatingVelocity();

		float newSepVelocity = -separatingVelocity * restitution;

		glm::vec2 accCausedVelocity(particle[0]->acceleration);
		if(particle[1])
			accCausedVelocity -= particle[1]->acceleration;

		float accCausedSepVelocity =  glm::dot(accCausedVelocity, contactNormal) * dt;

		if(accCausedSepVelocity < 0)
		{
			newSepVelocity += restitution * accCausedSepVelocity;

			if(newSepVelocity <= 0)
				newSepVelocity = 0;
			// TODO: ternary
		}
		
		float deltaVelocity = newSepVelocity - separatingVelocity;
		
		float totalInverseMass = particle[0]->getInverseMass();
		if(particle[1])
			totalInverseMass += particle[1]->getInverseMass();

		if(totalInverseMass <= 0)
			return;

		float impulse = deltaVelocity / totalInverseMass;
		glm::vec2 impulsePerIMass = contactNormal * impulse;
		particle[0]->velocity +=
			impulsePerIMass * particle[0]->getInverseMass();
		
		if(particle[1])
		{
			particle[1]->velocity +=
				impulsePerIMass * -particle[1]->getInverseMass();
		}
	}
	
	void Particle2DContact::resolveInterpenetration(float dt)
	{
		if(penetration <= 0)
			return;
		float totalInverseMass = particle[0]->getInverseMass();
		if(particle[1])
			totalInverseMass += particle[1]->getInverseMass();

		if(totalInverseMass <= 0)
			return;

		glm::vec2 movePerIMass = contactNormal * (penetration / totalInverseMass);
		particleMovement[0] = movePerIMass * particle[0]->getInverseMass();
		
		if(particle[1])
		{
			particleMovement[1] = movePerIMass * -particle[1]->getInverseMass();
		}
		else
		{
			particleMovement[1].x = 0;
			particleMovement[1].y = 0;
		}

		particle[0]->position += particleMovement[0];
		if(particle[1])
		{
			particle[1]->position += particleMovement[1];
		}
	}
}