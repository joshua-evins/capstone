#include "Node.h"
#include "Connection.h"

namespace Engine
{
namespace Editing
{
	Node::Node()
	{
		numConnections = 0;
	}

	void Node::setPosition(glm::vec3 newPos)
	{
		position = newPos;
	}
	
	void Node::addConnection(uint nodeIndex, float cost)
	{
		connections.push_back(new Connection(nodeIndex, cost));
		numConnections++;
	}
}
}