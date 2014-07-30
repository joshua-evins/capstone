#include "ShapeDataRead.h"
#include <fstream>
#include <ShapeData.h>

namespace Engine
{
	void ShapeDataRead::GameData::cleanup()
	{
		numShapeDatas = 0;
		delete [] bufferAddress;
	}

	ShapeDataRead::GameData ShapeDataRead::readShapeDatas(const char* fileName)
	{
		GameData retVal;
		

		std::ifstream in(fileName, std::ios::in | std::ios::binary);

		in.seekg(0, std::ios::end);
		const unsigned int bufferSize = (unsigned int)(in.tellg());

		char* buf = new char[bufferSize];
		
		in.seekg(0, std::ios::beg);
		
		in.read(buf, bufferSize);
		in.close();

		retVal.numShapeDatas = *(reinterpret_cast<unsigned int*>(buf));
		retVal.shapeDatas = reinterpret_cast<Neumont::ShapeData*>(buf + sizeof(unsigned int));

		for(unsigned int i=0; i<retVal.numShapeDatas; i++)
		{
			Neumont::ShapeData& d = retVal.shapeDatas[i];
			d.verts = reinterpret_cast<Neumont::Vertex*>(buf + (unsigned int)(d.verts));
			d.indices = reinterpret_cast<ushort*>(buf + (unsigned int)(d.indices));
		}
		retVal.bufferAddress = buf;

		return retVal;
	}
}
