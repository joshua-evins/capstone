#include "LevelData.h"
#include "Node.h"
#include "Connection.h"
#include <fstream>
#include <ShapeData.h>
#include <FbxLoader.h>
#include <iostream>

namespace Engine
{
namespace Editing
{
	// start with empty data containers
	LevelData::LevelData()
	{
		totalNodes = 0;
		totalShapeDatas = 0;
	}

	// hopefully get to this part
	LevelData::~LevelData()
	{

	}
		
	void LevelData::addNode(glm::vec3 position)
	{
		Engine::Editing::Node* node = new Node;
		node->setPosition(position);
		node->numConnections=0;
		nodes.push_back(node);
		totalNodes++;
	}

	uint LevelData::populateShapeDatas(uint numShapeDatas, char* data)
	{
		uint shapeDataSize = sizeof(Neumont::ShapeData);
		uint segmentSize = numShapeDatas*shapeDataSize;

		for(int i=0; i<numShapeDatas; i++)
		{
			shapeDatas.push_back(reinterpret_cast<Neumont::ShapeData*>(data + i*shapeDataSize));
		}

		for(unsigned int i=0; i<numShapeDatas; i++)
		{
			Neumont::ShapeData& d = *(shapeDatas.at(i));
			segmentSize += d.vertexBufferSize() + d.indexBufferSize();
			std::cout << "vbuffersz in popsd: " << d.vertexBufferSize() << std::endl;
			std::cout << "ibuffersz in popsd: " << d.indexBufferSize() << std::endl;
		}

		return segmentSize;
	}

	// use to import data from an fbx
	void LevelData::read_fbx(const char* fileName)
	{
		Neumont::ShapeData* shapeDatas = NULL;
		unsigned int numShapeDatas = 0;

		Neumont::FbxLoader::loadFbxFile(fileName, shapeDatas, numShapeDatas);

		totalShapeDatas+=numShapeDatas;
		populateShapeDatas(numShapeDatas, reinterpret_cast<char*>(shapeDatas));
	}

	// use for the first read
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
		
	// read in this huge chunk, send down send it down to gl, create renderables, etc.
	void LevelData::read_temp_v1(std::ifstream* stream, uint size)
	{
		char* data = new char[size];
		stream->read(data, size);
		
		read_segment_shapeData_v1(data);

		//delete [] data;
	}
		
	uint LevelData::read_segment_shapeData_v1(char* data)
	{
		uint segmentSize = 0;
		uint oldTotal = totalShapeDatas;
		uint numShapeDatasIn = *(reinterpret_cast<uint*>(data));
		segmentSize += sizeof(uint);

		Neumont::ShapeData* shapeDatasIn = reinterpret_cast<Neumont::ShapeData*>(data+sizeof(uint));

		for(uint i=0; i<numShapeDatasIn; i++)
		{
			Neumont::ShapeData& d = shapeDatasIn[i];
			d.verts = reinterpret_cast<Neumont::Vertex*>(data + (uint)(d.verts));
			d.indices = reinterpret_cast<ushort*>(data + (uint)(d.indices));
			shapeDatas.push_back(shapeDatasIn+i);
			totalShapeDatas++;
			segmentSize += sizeof(Neumont::ShapeData) + d.vertexBufferSize() + d.indexBufferSize();
		}

		return segmentSize;
	}

	// read chunk, add the nodes, add the connections
	// make renderables for them
	void LevelData::read_lasting_v1(std::ifstream* stream, uint size)
	{
		char* data = new char[size];
		stream->read(data, size);

		read_segment_nodes_v1(data);
	}
		
	uint LevelData::read_segment_nodes_v1(char* data)
	{
		uint formerTotalNodes = totalNodes;
		uint segmentSize = 0;

		uint numNodesIn = *(reinterpret_cast<uint*>(data));
		segmentSize += sizeof(uint);

		char* nodesStart = data + sizeof(uint);

		// position, connections, numConnections
		uint binNodeSize = sizeof(glm::vec3) + sizeof(Connection*) + sizeof(uint);
		segmentSize += binNodeSize * numNodesIn;

		char* connsStart = nodesStart + numNodesIn * binNodeSize;
		uint numPreviousConns = 0;
		for(uint i=0; i<numNodesIn; i++)
		{
			Engine::Editing::Node* n = new Node;
			
			n->position = *(reinterpret_cast<glm::vec3*>(nodesStart + binNodeSize*i));
			char* numConn = nodesStart + (binNodeSize*i + sizeof(glm::vec3) + sizeof(Node*));
			uint* nC = reinterpret_cast<uint*>(numConn);
			n->numConnections = *nC;
			for(int j=0; j<n->numConnections; j++)
			{
				Engine::Editing::Connection* conn =
					reinterpret_cast<Connection*>(
						connsStart + numPreviousConns*sizeof(Connection)
					);
				conn->nodeIndex += formerTotalNodes;
				n->connections.push_back(conn);
				numPreviousConns++;
			}

			nodes.push_back(n);

			segmentSize += sizeof(Connection) * n->numConnections;

			totalNodes++;
		}

		return segmentSize;
	}





	void LevelData::write_v1(const char* fileName)
	{
		uint metaDataSize_v1 = 3*sizeof(uint);
		uint currentOffset = 0;

		std::ofstream out(fileName, std::ios::out | std::ios::binary);
		
		out.seekp(metaDataSize_v1, std::ios::beg);

		uint tempSize = write_temp_v1(&out);
		currentOffset += tempSize;

		uint lastingSize = write_lasting_v1(&out);
		currentOffset += lastingSize;

		out.seekp(std::ios::beg);
		write_metaData_v1(&out, tempSize, lastingSize);
		out.close();
	}
	
	void LevelData::write_metaData_v1(std::ofstream* stream, uint tempSize, uint lastingSize)
	{
		uint version = 8;
		stream->write(reinterpret_cast<char*>(&version), sizeof(uint));

		stream->write(reinterpret_cast<char*>(&tempSize), sizeof(uint));

		stream->write(reinterpret_cast<char*>(&lastingSize), sizeof(uint));
	}

	uint LevelData::write_temp_v1(std::ofstream* stream)
	{
		uint tempSize = 0;

		tempSize += write_segment_shapeDatas(stream);

		return tempSize;
	}

	uint LevelData::write_lasting_v1(std::ofstream* stream)
	{
		uint lastingSize = 0;

		lastingSize += write_segment_nodes(stream);

		return lastingSize;
	}

	uint LevelData::write_segment_shapeDatas(std::ofstream* stream)
	{
		uint segmentStart = (uint)(stream->tellp());
		uint segmentLength = 0;

		stream->write(reinterpret_cast<char*>(&totalShapeDatas), sizeof(uint));

		uint dataStart = (uint)(stream->tellp());

		Neumont::ShapeData* sdCopy = new Neumont::ShapeData[totalShapeDatas];
		uint shapeDataSize = totalShapeDatas * sizeof(Neumont::ShapeData);
		
		
		// copy the source shapedata data into the clone by loop because std::vector isn't a flat pointer
		for(int i=0; i<totalShapeDatas; i++)
		{
			Neumont::ShapeData* original = shapeDatas.at(i);
			memcpy(sdCopy+i, original, sizeof(Neumont::ShapeData));
		}

		stream->seekp(shapeDataSize + dataStart, std::ios::beg);

		// convert pointers to offsets
		uint offset = 0;
		for(int i=0; i<totalShapeDatas; i++)
		{
			Neumont::ShapeData& d = sdCopy[i];
			
			offset = (uint)(stream->tellp());
			offset -= segmentStart;
			stream->write(reinterpret_cast<char*>(d.verts), d.vertexBufferSize());
			memcpy(&(d.verts), &offset, sizeof(uint));

			offset = (uint)(stream->tellp());
			offset -= segmentStart;
			stream->write(reinterpret_cast<char*>(d.indices), d.indexBufferSize());
			memcpy(&(d.indices), &offset, sizeof(uint));
		}
		uint dataEnd = (uint)(stream->tellp());
		segmentLength = dataEnd - segmentStart;

		stream->seekp(dataStart, std::ios::beg);

		stream->write(reinterpret_cast<char*>(sdCopy), shapeDataSize);
		delete [] sdCopy;

		stream->seekp(dataEnd, std::ios::beg);

		return segmentLength;
	}

	uint LevelData::write_segment_nodes(std::ofstream* stream)
	{
		uint segmentLength = 0;

		stream->write(reinterpret_cast<char*>(&totalNodes), sizeof(uint));
		segmentLength += sizeof(uint);

		// write each node
		uint runningOffset = 0;
		for(int i=0; i<totalNodes; i++)
		{
			Node& n = *(nodes.at(i));
			stream->write(reinterpret_cast<char*>(&(n.position)), sizeof(glm::vec3));
			stream->write(reinterpret_cast<char*>(&(runningOffset)), sizeof(uint));
			stream->write(reinterpret_cast<char*>(&(n.numConnections)), sizeof(uint));
			runningOffset += n.numConnections * sizeof(Connection);
			segmentLength += sizeof(glm::vec3) + sizeof(uint)*2;
		}

		for(int i=0; i<totalNodes; i++)
		{
			Node& n = *(nodes.at(i));
			for(int j=0; j<n.numConnections; j++)
			{
				Connection& c = *(n.connections.at(j));
				stream->write(reinterpret_cast<char*>(&(c.nodeIndex)), sizeof(uint));
				stream->write(reinterpret_cast<char*>(&(c.cost)), sizeof(float));
				segmentLength += 2*sizeof(uint);
			}
		}

		return segmentLength;
	}
}
}