#pragma once
#include "ExportHeader.h"
#include "typedefs_Engine.h"

namespace Engine
{
	struct Node;
	struct Connection;
	struct ENGINE_SHARED PathConnection
	{
		uint nodeIndex;
		float cost;
		Node* parent;
		void initialize(Connection* c, Node* parent);
	};

}