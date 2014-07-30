#ifndef SHAPEDATAREAD_H
#define SHAPEDATAREAD_H
#include "ExportHeader.h"

namespace Neumont{ struct ShapeData; }

namespace Engine
{
	struct ENGINE_SHARED ShapeDataRead
	{
		struct ENGINE_SHARED GameData
		{
			Neumont::ShapeData* shapeDatas;
			unsigned int numShapeDatas;
			char* bufferAddress;
			void cleanup();
		};
		public: static GameData readShapeDatas(const char* fileName);
	};
}

#endif