#ifndef GEOMETRY_TOOL_DATA_H
#define GEOMETRY_TOOL_DATA_H
#include <vector>

template<class TVert>
struct GeometryToolData
{
	size_t textureIndex;
	size_t normalMapIndex;
	size_t numVerts;
	TVert* vertices;
	size_t numIndices;
	unsigned short* indices;
	inline GeometryToolData() : vertices(NULL), indices(NULL), textureIndex(-1), normalMapIndex(-1) {}
	GeometryToolData<TVert> deepCopy() const
	{
		GeometryToolData<TVert> ret;
		ret.textureIndex = textureIndex;
		ret.numVerts = numVerts;
		ret.numIndices = numIndices;
		if(vertices != NULL)
		{
			ret.vertices = new TVert[numVerts];
			memcpy(ret.vertices, vertices, sizeof(TVert) * numVerts);
		}
		if(indices != NULL)
		{
			ret.indices = new unsigned short[numIndices];
			memcpy(ret.indices, indices, sizeof(unsigned short) * numIndices);
		}
		return ret; // This causes a shallow copy, but local copy is lost so OK
	}
	void destroy()
	{
		delete [] vertices;
		vertices = NULL;
		delete [] indices;
		indices = NULL;
	}
};

#endif