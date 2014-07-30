#pragma once
#include "ShapeData.h"
#include "../ExportHeader.h"
#include <vector>
#include <glm\glm.hpp>

namespace Engine
{
namespace Editing
{
	struct Node;
	class ENGINE_SHARED LevelData
	{
	public:
		uint totalNodes;
		std::vector<Node*> nodes;

		// Geometry
		uint totalShapeDatas;
		std::vector<Neumont::ShapeData*> shapeDatas;
		void addNode(glm::vec3 position);
		// start with empty data containers
		LevelData();

		// hopefully get to this part
		~LevelData();

		uint populateShapeDatas(uint numShapeDatas, char* data);

		// use to import native binary data
		void read_native(const char* fileName);

		void read_native_v1(std::ifstream* stream);

		// use to import data from an fbx
		void read_fbx(const char* fileName);
		
		// read in this huge chunk, send down send it down to gl, create renderables, etc.
		void read_temp_v1(std::ifstream* stream, uint size);
		
		uint read_segment_shapeData_v1(char* data);

		// read chunk, add the nodes, add the connections
		// make renderables for them
		void read_lasting_v1(std::ifstream* stream, uint size);
		
		uint read_segment_nodes_v1(char* data);
		


		void write_v1(const char* fileName);

		void write_metaData_v1(std::ofstream* stream, uint tempSize, uint lastingSize);

		uint write_temp_v1(std::ofstream* stream);
		uint write_lasting_v1(std::ofstream* stream);

		uint write_segment_shapeDatas(std::ofstream* stream);
		uint write_segment_nodes(std::ofstream* stream);
	};
}
}