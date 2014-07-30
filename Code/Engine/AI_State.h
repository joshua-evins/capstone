#pragma once
#include "ExportHeader.h"
#include "typedefs_Engine.h"

namespace Engine
{
	struct ENGINE_SHARED AI_State
	{
		uint type;
		virtual void update();
	};
}