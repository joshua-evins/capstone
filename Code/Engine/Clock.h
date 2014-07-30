#ifndef CLOCK_H
#define CLOCK_H
#define  WIN32_LEAN_AND_MEAN
#include "ExportHeader.h"
#include "typedefs_Engine.h"
#include <windows.h>

namespace Engine
{
	class ENGINE_SHARED Clock
	{

	#pragma region Instance Variables
		LARGE_INTEGER clockFrequency;
		LARGE_INTEGER lastStamp;
	#pragma endregion

	public:
		Clock();

	#pragma region Timing Functions
		void frameTick();
		float deltaTime();
	#pragma endregion
	
	};
}
#endif