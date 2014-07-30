#include "Heuristic.h"
#include "Node.h"

namespace Engine
{
namespace Editing
{
	float Heuristic::estimate(Node* n)
	{
		return glm::length(n->position - end->position);
	}
}
}