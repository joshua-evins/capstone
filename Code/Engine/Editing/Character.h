#pragma once
#include "..\Renderer.h"
#include "..\ExportHeader.h"
#include "glm\glm.hpp"


namespace Engine
{
namespace Editing
{
	struct Node;
	struct ENGINE_SHARED Character
	{
		Character();
		
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
		bool path();
		void faster();
		void slower();
	};
}
}