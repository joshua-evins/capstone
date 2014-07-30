#pragma once
#include "ExportHeader.h"
namespace Engine
{
	struct Node;
	struct ENGINE_SHARED Heuristic
	{
		Node* end;
		float estimate(Node* n);
		virtual float inheritedEstimate(Node* n);
	};
}