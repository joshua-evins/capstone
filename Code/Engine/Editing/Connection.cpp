#include "Connection.h"

namespace Engine
{
namespace Editing
{
	Connection::Connection(uint nodeIndex, float cost)
	{
		this->nodeIndex = nodeIndex;
		this->cost = cost;
	}
}
}