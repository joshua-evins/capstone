#include "Connection.h"

namespace Engine
{
	Connection::Connection(uint nodeIndex, float cost)
	{
		this->nodeIndex = nodeIndex;
		this->cost = cost;
	}
}