#pragma once
#include <glm\glm.hpp>
#include "ExportHeader.h"

namespace Engine
{
	class ENGINE_SHARED Particle2D
	{
	#pragma region Constructors and Destructors
		public:
			Particle2D();
			~Particle2D();
	#pragma endregion

	#pragma region Instance Variables
		public:
			glm::vec2 position;
			glm::vec2 velocity;
			glm::vec2 acceleration;
			float damping;
			bool active;
		protected:
			glm::vec2 impulseSum;
			float inverseMass;
	#pragma endregion
		
	#pragma region inverseMass Accessors and Mutators
		public:
			float getInverseMass();
			void setInverseMass(float inverseMass);
			float getMass();
			void setMass(float mass);
	#pragma endregion

	public:
		float getMassRadius();

	#pragma region Impulse and Force
		public:
			void addImpulse(glm::vec2 impulse);
			void applyForce(glm::vec2 force);
		private:
			void applyImpulseSum(float dt);
			void clearImpulseSum();
	#pragma endregion

	public:
		void integrate(float dt);
	};
}
