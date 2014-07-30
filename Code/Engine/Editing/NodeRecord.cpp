#include "NodeRecord.h"
#include "PathConnection.h"

namespace Engine
{
namespace Editing
{

	void NodeRecord::cleanup()
	{
		delete connection;
	}
}
}