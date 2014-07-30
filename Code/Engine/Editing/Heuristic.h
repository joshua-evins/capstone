#pragma once
#include "..\ExportHeader.h"
namespace Engine
{
namespace Editing
{
	struct Node;
	struct ENGINE_SHARED Heuristic
	{
		Node* end;
		float estimate(Node* n);
	};
}
}