#pragma once

#include"EngineCore.h"

namespace Czuch
{

#define FPS30_TIME 1.0f/30.0f
#define FPS60_TIME 1.0f/60.0f


	class Time
	{
	public:

		static F32 DeltaTime;

#ifdef CZUCH_PLATFORM_WINDOWS
		inline static void ReadHiResFrequency(LARGE_INTEGER& inVal) {
			QueryPerformanceFrequency(&inVal);
		}

		inline static void ReadHiResCounter(LARGE_INTEGER& inVal)
		{
			QueryPerformanceCounter(&inVal);
		}
#else
		inline static void ReadHiResFrequency(LARGE_INTEGER& inVal) {

		}

		inline static void ReadHiResCounter(LARGE_INTEGER& inVal)
		{

		}
#endif

	};

}