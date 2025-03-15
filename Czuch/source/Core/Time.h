#pragma once

#include"EngineCore.h"
#ifdef CZUCH_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace Czuch
{

#define FPS30_TIME 1.0f/30.0f
#define FPS60_TIME 1.0f/60.0f


	class CZUCH_API Time
	{
	private:
		static F32 m_TimeFromStart;
	public:

		friend class EngineRoot;
		static F32 DeltaTime;
		static F32 TimeFromStart() { return m_TimeFromStart; }


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