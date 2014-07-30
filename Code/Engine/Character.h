#pragma once
#include <glm\glm.hpp>
#include "ExportHeader.h"
#include "typedefs_Engine.h"
#include "Renderer.h"
#include "AI_State.h"
#include "Flag.h"
#include "Node.h"

namespace Engine
{
	struct ENGINE_SHARED Character
	{
		Engine::Flag* enemyFlag;
		Character();
		bool hasFlag;
		AI_State* currentState;

		glm::vec3 position;
		glm::vec3 facing;
		float speed;

		uint flagNodeIndex;
		uint enemyFlagNodeIndex;

		Renderer::Renderable* renderable;
		Renderer::UniformInfo* transform;

		Node** nodePath;
		uint totalNodes;
		uint nextNode;

		void face(glm::vec3 thisWay);
		void startPath(Node** newNodes, uint numNodes);
		bool needsPathUpdate();
		void update();
		bool path();
		void faster();
		void slower();
	};
}