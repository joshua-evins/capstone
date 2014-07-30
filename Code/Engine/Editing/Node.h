#pragma once
#include <glm\glm.hpp>
#include <vector>
#include "../typedefs_Engine.h"
#include "../ExportHeader.h"

namespace Engine
{
namespace Editing
{
	struct Connection;
	struct ENGINE_SHARED Node
	{
		glm::vec3 position;
		std::vector<Connection*> connections;
		uint numConnections;
		Node();
		void setPosition(glm::vec3 newPos);
		void addConnection(uint nodeIndex, float cost);
	};
}
}