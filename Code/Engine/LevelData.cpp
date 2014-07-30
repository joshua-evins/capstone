#include "LevelData.h"
#include "Node.h"
#include "Connection.h"
#include <fstream>
#include <ShapeData.h>
#include <FbxLoader.h>
#include <iostream>

namespace Engine
{
	LevelData::LevelData()
	{
		totalNodes = 0;
		totalShapes = 0;
		shapeDeletionPointer = NULL;
	}

	LevelData::~LevelData()
	{

	}

	void LevelData::read_native(const char* fileName)
	{	
		std::ifstream in(fileName, std::ios::in | std::ios::binary);
		
		uint version = 0;
		in.read(reinterpret_cast<char*>(&version), sizeof(uint));
		
		if(version==8) // shapeDatas and nodes
		{
			read_native_v1(&in);
		}
		in.close();
	}

	void LevelData::read_native_v1(std::ifstream* stream)
	{
		// read MetaData
		uint metaDataSize = sizeof(uint)*2;
		char* metaBuf = new char[metaDataSize];
		stream->read(metaBuf, metaDataSize);
		uint tempSize = *(reinterpret_cast<uint*>(metaBuf)+0);
		uint lastingSize = *(reinterpret_cast<uint*>(metaBuf)+1);
		
		// run read functions
		read_temp_v1(stream, tempSize);
		// read functions must leave the read pointer at the end of their chunk
		read_lasting_v1(stream, lastingSize);
	}
		
	void LevelData::read_temp_v1(std::ifstream* stream, uint size)
	{
		char* data = new char[size];
		stream->read(data, size);
		
		read_segment_shapeData_v1(data);
	}
		
	void LevelData::read_segment_shapeData_v1(char* data)
	{
		shapeDeletionPointer = data;

		totalShapes = *(reinterpret_cast<uint*>(data));

		shapes = reinterpret_cast<Neumont::ShapeData*>(data+sizeof(uint));

		for(uint i=0; i<totalShapes; i++)
		{
			Neumont::ShapeData& d = shapes[i];
			d.verts = reinterpret_cast<Neumont::Vertex*>(data + (uint)(d.verts));
			d.indices = reinterpret_cast<ushort*>(data + (uint)(d.indices));
		}
	}

	void LevelData::read_lasting_v1(std::ifstream* stream, uint size)
	{
		char* data = new char[size];
		stream->read(data, size);

		read_segment_nodes_v1(data);
	}
		
	void LevelData::read_segment_nodes_v1(char* data)
	{
		totalNodes = *(reinterpret_cast<uint*>(data));

		char* nodesStart = data + sizeof(uint);
		char* connsStart = nodesStart + totalNodes * sizeof(Node*);
		nodes = reinterpret_cast<Node*>(nodesStart);

		for(uint i=0; i<totalNodes; i++)
		{
			Engine::Node* n = nodes+i;
			uint offsetFromConnStart = *(reinterpret_cast<uint*>(&(n->connections)));
			n->connections = reinterpret_cast<Connection*>(connsStart +  offsetFromConnStart);
		}
	}
}