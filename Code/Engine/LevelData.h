#pragma once
#include "ShapeData.h"
#include "ExportHeader.h"
#include <vector>
#include <glm\glm.hpp>

namespace Engine
{
	struct Node;
	class ENGINE_SHARED LevelData
	{
	public:
		Node* nodes;
		uint totalNodes;

		char* shapeDeletionPointer;
		Neumont::ShapeData* shapes;
		uint totalShapes;

		LevelData();

		// hopefully get to this part
		~LevelData();

		uint populateShapeDatas(uint numShapeDatas, char* data);

		// use to import native binary data
		void read_native(const char* fileName);

		void read_native_v1(std::ifstream* stream);
		
		// read in this huge chunk, send down send it down to gl, create renderables, etc.
		void read_temp_v1(std::ifstream* stream, uint size);
		
		void read_segment_shapeData_v1(char* data);

		// read chunk, add the nodes, add the connections
		// make renderables for them
		void read_lasting_v1(std::ifstream* stream, uint size);
		
		void read_segment_nodes_v1(char* data);
	};
}