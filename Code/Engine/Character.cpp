#include "Character.h"
#include <glm\gtx\transform.hpp>

namespace Engine
{
	Character::Character()
	{
		enemyFlag = NULL;
		hasFlag = false;

		speed = 0.1f;

		renderable = NULL;
		transform = NULL;

		nodePath = NULL;
		totalNodes = 0;
		nextNode = 0;
	}

	void Character::face(glm::vec3 newdir)
	{
		facing = newdir;
		if(renderable != NULL && transform != NULL)
		{
			//arrow vertices point along adir
			glm::vec3 adir(0.0f,0.0f,-1.0f);

			//arrow should point along newdir
			float angle = glm::dot(adir, newdir);
			angle = glm::degrees(acosf(angle));

			glm::vec3 cross = glm::cross(adir, newdir);

			glm::mat4 translate = glm::translate(position);
			glm::mat4 rot = glm::rotate(angle, cross);
			glm::mat4 xform = translate * rot;

			transform->updateInfo(64, reinterpret_cast<char*>(&xform));
		}
	}

	void Character::startPath(Node** newNodes, uint numNodes)
	{
		if(numNodes > 0)
		{
			nodePath = newNodes;
			totalNodes = numNodes;
			nextNode = 0;
		}
	}

	void Character::update()
	{
		path();
		if(hasFlag)
		{
			enemyFlag->position = position;
		}
	}

	bool Character::path()
	{
		if(totalNodes < 1 || nodePath == NULL) // not pathing
		{
			//update path
			return false;
		}
		// I have a path

		Node* node = nodePath[nextNode];
		// am I at the next node?
		if(glm::length(node->position - position) < speed) // I am at the next node
		{
			nextNode++; // go to the next node
			if(nextNode >= totalNodes) // that was the last node
			{
				totalNodes = 0;
				nodePath = NULL;
				return false;
			}
		}
		else
		{
			node = nodePath[nextNode];
			facing = glm::normalize(node->position - position);
			position += facing * speed;
			face(facing);
		}
		return true;
	}

	void Character::faster()
	{
		speed = (speed + 0.01f > 1.0f) ? speed : speed + 0.01f;
	}

	void Character::slower()
	{
		speed = (speed - 0.01f < 0.02f) ? speed : speed - 0.01f;
	}
}