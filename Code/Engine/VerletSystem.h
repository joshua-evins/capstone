#pragma once
#define VS_NUMPOINTS 100
#define VS_NUMITERATIONS 1
#define VS_NUMABSOLUTES 10
#define VS_POINTSWIDE 10
#define VS_NUMCONSTRAINTS 400
#define VS_RESTLENGTH 4
#define VS_TOPEDGE 20

#include <glm\glm.hpp>

namespace Engine
{
	class VerletSystem
	{
	public:
		glm::vec2 currentPos[VS_NUMPOINTS];
		glm::vec2 lastPos[VS_NUMPOINTS];
		struct Constraint
		{
			int p1, p2;
			float restLength;

		} constraints[400];

		int currentConstraint;

	private:
		
		glm::vec2 acceleration[VS_NUMPOINTS];
		glm::vec2 gravity;
		float dt;

		struct AbsoluteConstraint
		{
			int index;
			glm::vec2 position;

		} absolutes[VS_NUMABSOLUTES];
		
		void MakeAbsolutes();
		void MakeConstraints();

	public:
		void AccumulateForces();
		void Integrate();
		void SatisfyConstraints();

	public:
		VerletSystem(float dt);
		~VerletSystem();
		void TimeStep();
		int closestParticle(glm::vec2);
		void setPosition(int index, glm::vec2 mousePos);
	};
}