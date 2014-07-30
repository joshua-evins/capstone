#include "Heuristic.h"
#include "Node.h"

namespace Engine
{
	float Heuristic::estimate(Node* n)
	{
		return glm::length(n->position - end->position) + inheritedEstimate(n);
	}

	float Heuristic::inheritedEstimate(Node* )
	{
		return 0.0f;
	}
}