#pragma once
#include "ExportHeader.h"
#include "typedefs_Engine.h"
#include "NodeRecord.h"
#include <vector>

namespace Engine
{
	struct NodeRecord;
	struct Node;
	struct ENGINE_SHARED PathfindingList
	{
		std::vector<NodeRecord*> records;
	
		void cleanup();
		void add(NodeRecord* record);
		void remove(NodeRecord* record);
		NodeRecord* smallest();
		NodeRecord* find(Node* searchNode);
		bool contains(Node* searchNode);
		uint numRecords();
	};
}