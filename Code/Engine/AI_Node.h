#pragma once
#include "ExportHeader.h"
#include "AI_State.h"

namespace Engine
{
	struct ENGINE_SHARED AI_Node
	{
	public:
		AI_Node* trueNode;
		AI_Node* falseNode;
		
		virtual AI_State* check();
	};
}
