#pragma once
#include "../typedefs_Engine.h"
#include "../ExportHeader.h"

namespace Engine
{
namespace Editing
{
	struct ENGINE_SHARED Connection
	{
		uint nodeIndex;
		float cost;
		Connection(uint nodeIndex, float cost);
	};
}
}