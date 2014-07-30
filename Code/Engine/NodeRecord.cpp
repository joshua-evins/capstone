#include "NodeRecord.h"
#include "PathConnection.h"
namespace Engine
{
	void NodeRecord::cleanup()
	{
		delete connection;
	}
}