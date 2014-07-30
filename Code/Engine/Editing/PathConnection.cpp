#include "PathConnection.h"
#include "Node.h"
#include "Connection.h"

namespace Engine
{
namespace Editing
{
	void PathConnection::initialize(Connection* c, Node* parent)
	{
		nodeIndex = c->nodeIndex;
		cost = c->cost;
		this->parent = parent;
	}
}
}