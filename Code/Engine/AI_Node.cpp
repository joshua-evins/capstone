#include "AI_Node.h"

namespace Engine
{
	AI_State* AI_Node::check()
	{
		return new AI_State;
	}
}