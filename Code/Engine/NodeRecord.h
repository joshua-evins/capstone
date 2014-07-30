#pragma once
#include "ExportHeader.h"
namespace Engine
{
	struct Node;
	struct PathConnection;
	struct ENGINE_SHARED NodeRecord
	{
		Node* node;
		PathConnection* connection;
		float costSoFar;
		float estimatedTotalCost;
		void cleanup();
	};

}