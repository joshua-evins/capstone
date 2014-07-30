#include "PathfindingList.h"
#include "NodeRecord.h"
#include "Node.h"

namespace Engine
{
namespace Editing
{

	void PathfindingList::cleanup()
	{
		for(uint i=0; i<records.size(); i++)
		{
			records.at(i)->cleanup();
			delete records.at(i);
		}
	}

	void PathfindingList::add(NodeRecord* record)
	{
		records.push_back(record);
	}

	void PathfindingList::remove(NodeRecord* record)
	{
		uint foundAt = records.size();
		for(uint i=0; i<records.size() && foundAt==records.size(); i++)
		{
			if(records.at(i) == record)
			{
				foundAt=i;
			}
		}
		if(foundAt != records.size())
		{
			records.erase(records.begin() + foundAt);
		}
	}

	NodeRecord* PathfindingList::smallest()
	{
		float lowestDistance = FLT_MAX;
		uint lowestIndex = records.size();
		for(uint i=0; i<records.size(); i++)
		{
			if(records.at(i)->estimatedTotalCost < lowestDistance)
			{
				lowestDistance = records.at(i)->estimatedTotalCost;
				lowestIndex = i;
			}
		}
		// TODO
		return records.at(lowestIndex);
	}

	NodeRecord* PathfindingList::find(Node* searchNode)
	{
		uint foundAt = records.size();
		for(uint i=0; i<records.size() && foundAt==records.size(); i++)
		{
			if(records.at(i)->node == searchNode)
			{
				foundAt=i;
			}
		}
		return (foundAt < records.size()) ? records.at(foundAt) : NULL;
	}

	bool PathfindingList::contains(Node* searchNode)
	{
		NodeRecord* nr = find(searchNode);
		return  nr != NULL;
	}

	uint PathfindingList::numRecords()
	{
		return (uint)(records.size());
	}

}
}