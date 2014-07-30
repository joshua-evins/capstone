#include "VerletSystem.h"

namespace Engine
{
	VerletSystem::VerletSystem(float dt)
	{
		this->dt = dt;
		currentConstraint = 0;
		gravity = glm::vec2(1.0f, -10.0f);

		for(int i=0; i<VS_NUMPOINTS; i++)
		{
			currentPos[i].x = 30.0f - i%VS_POINTSWIDE * VS_RESTLENGTH * 1.05f;
			currentPos[i].y = 30.0f - i/VS_POINTSWIDE * VS_RESTLENGTH * 1.05f;

			lastPos[i] = currentPos[i];
		}

		MakeAbsolutes();
		MakeConstraints();
	}

	#pragma region Correct
	VerletSystem::~VerletSystem()
	{
	}

	void VerletSystem::AccumulateForces()
	{
		for(int i=0; i<VS_NUMPOINTS; i++)
			acceleration[i] = gravity;
	}

	void VerletSystem::Integrate()
	{
		for(int i=0; i<VS_NUMPOINTS; i++)
		{
			glm::vec2& currentP = currentPos[i];
			glm::vec2 temp = currentPos[i];

			currentPos[i] += currentPos[i] - lastPos[i] + acceleration[i]*dt*dt;
			lastPos[i] = temp;
		}
	}

	void VerletSystem::SatisfyConstraints()
	{
		for(int j=0; j<VS_NUMITERATIONS; j++) // probably 1
		{
			for(int i=currentConstraint-1; i>=0; i--) // inverting the i and j here is clever
			{
				Constraint& c = constraints[i];
				glm::vec2& pos1 = currentPos[c.p1];
				glm::vec2& pos2 = currentPos[c.p2];
				
				glm::vec2 diff(pos1 - pos2);
				float d = glm::length(diff);

				float difference = (c.restLength - d)/d;

				glm::vec2 translate(diff * 0.5f * difference);

				pos1 += translate;
				pos2 -= translate;
				currentPos[c.p1] = pos1;
				currentPos[c.p2] = pos2;
				//lastPos[c.p1] = pos1;
				//lastPos[c.p2] = pos2;
				for(int k=0; k<VS_NUMABSOLUTES; k++)
				{
				currentPos[absolutes[k].index] = absolutes[k].position;
				//lastPos[absolutes[i].index] = absolutes[i].position;
			}
			}
			
		}
	}

	void VerletSystem::TimeStep()
	{
			AccumulateForces();
			Integrate();
			SatisfyConstraints();
	}

	void VerletSystem::MakeAbsolutes()
	{
		for(int i=0; i<VS_NUMABSOLUTES; i++)
		{
			absolutes[i].index = i;
			absolutes[i].position = currentPos[i];
		}
	}

	void VerletSystem::MakeConstraints()
	{
		for(int i=0; i<VS_NUMPOINTS; i++)
		{
			if(i<VS_NUMPOINTS-VS_POINTSWIDE) //connect down
			{
				constraints[currentConstraint].p1 = i;
				constraints[currentConstraint].p2 = i+VS_POINTSWIDE;
				constraints[currentConstraint].restLength = VS_RESTLENGTH;
				currentConstraint++;
			}
			if(i%VS_POINTSWIDE < VS_POINTSWIDE-1) // connect right
			{
				constraints[currentConstraint].p1 = i;
				constraints[currentConstraint].p2 = i+1;
				constraints[currentConstraint].restLength = VS_RESTLENGTH;
				currentConstraint++;
			}
			//if(i%VS_POINTSWIDE < VS_POINTSWIDE-1 && i<VS_NUMPOINTS-VS_POINTSWIDE) // connect bottom-right
			//{
			//	constraints[currentConstraint].p1 = i;
			//	constraints[currentConstraint].p2 = i+VS_POINTSWIDE+1;
			//	constraints[currentConstraint].restLength = VS_RESTLENGTH * sqrt(1.0f);
			//	currentConstraint++;
			//}
			//if(i<VS_NUMPOINTS-VS_POINTSWIDE && i%VS_POINTSWIDE > 0) // connect bottom-left
			//{
			//	constraints[currentConstraint].p1 = i;
			//	constraints[currentConstraint].p2 = i+VS_POINTSWIDE-1;
			//	constraints[currentConstraint].restLength = VS_RESTLENGTH * sqrt(2.0f);
			//	currentConstraint++;
			//}
		}
	}
#pragma endregion

	int VerletSystem::closestParticle(glm::vec2 clickPos)
	{
		clickPos /= 10.0f; // VS_SCREENSCALE
		int retVal = -1;
		float maxDist = 10000.0f;
		for(int i=0; i<VS_NUMPOINTS; i++)
		{
			if(glm::distance(currentPos[i], clickPos) < maxDist)
			{
				retVal = i;
				maxDist = glm::distance(currentPos[i], clickPos);
			}
		}

		return retVal;
	}

	void VerletSystem::setPosition(int index, glm::vec2 mousePos)
	{
		currentPos[index] = mousePos / 10.0f;
	}
}