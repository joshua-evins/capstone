#pragma once
#include "ExportHeader.h"
#include <glm\glm.hpp>
#include "Renderer.h"

namespace Engine
{
	struct ENGINE_SHARED Flag
	{
		Flag();
		glm::vec3 position;
		Renderer::UniformInfo* transform;
	};
}