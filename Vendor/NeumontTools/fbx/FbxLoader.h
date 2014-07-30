#ifndef FBX_LOADER_H
#define FBX_LOADER_H
#include "../ExportImportHeader.h"
#include "../Vertex.h"
#include "../ShapeData.h"

namespace Neumont
{
	class DLL_SHARED FbxLoader
	{
		static bool averageNormals;
	public:
		static bool loadFbxFile(
			const char* filename,
			Neumont::ShapeData*& shapeDatas,
			unsigned int& numShapeDatas);
		static void ToggleAverage(bool);
	};
}

#endif