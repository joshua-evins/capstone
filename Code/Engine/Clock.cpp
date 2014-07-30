#include "Clock.h"

namespace Engine
{
	Clock::Clock()
	{
		QueryPerformanceFrequency( &clockFrequency );
		frameTick();
	}

#pragma region Timing Functions
	void Clock::frameTick()
	{
		QueryPerformanceCounter( &lastStamp );
	}

	float Clock::deltaTime()
	{
		LARGE_INTEGER currentStamp;
		QueryPerformanceCounter( &currentStamp );
		return (float)((double)(currentStamp.QuadPart - lastStamp.QuadPart) / clockFrequency.QuadPart);
	}
#pragma endregion

}