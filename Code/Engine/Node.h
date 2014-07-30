#pragma once
#include <glm\glm.hpp>
#include "typedefs_Engine.h"
#include "ExportHeader.h"

namespace Engine
{
	struct Connection;
	struct ENGINE_SHARED Node
	{
		glm::vec3 position;
		Connection* connections;
		uint numConnections;
		Node();
	};
}