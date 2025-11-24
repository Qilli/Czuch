#include "czpch.h"
#include "TimeDiffCounter.h"
#include "Time.h"

namespace Czuch
{

	TimeDiffCounter::TimeDiffCounter(Mode mode) : ticks(), selectedMode(mode)
	{
		LARGE_INTEGER freq;
		Time::ReadHiResFrequency(freq);
#ifdef CZUCH_PLATFORM_WINDOWS
		frequency = F64(freq.QuadPart);
#else
		frequency = F64(freq); // Na Macu to zwykły long long
#endif
		ApplyMode();
	}

	void TimeDiffCounter::ApplyMode()
	{
		if (selectedMode == Mode::Miliseconds)
		{
			frequency /= 1000.0;
		}
		else if (selectedMode == Mode::Microseconds)
		{
			frequency /= 1000000.0;
		}
	}

	void TimeDiffCounter::StartCounter()
	{
		LARGE_INTEGER ticksCurrent;
		Time::ReadHiResCounter(ticksCurrent);
#ifdef CZUCH_PLATFORM_WINDOWS
		ticks = U64(ticksCurrent.QuadPart);
#else
		ticks = U64(ticksCurrent);
#endif
	}

	F32 TimeDiffCounter::GetCounter(bool reset)
	{
		LARGE_INTEGER finalTicks;
		Time::ReadHiResCounter(finalTicks);
#ifdef CZUCH_PLATFORM_WINDOWS
		F32 result = (F32)((finalTicks.QuadPart - ticks) / frequency);
		if (reset)
		{
			ticks = finalTicks.QuadPart;
		}
		return result;
#else
		F32 result = (F32)((finalTicks - ticks) / frequency);
		if (reset)
		{
			ticks = finalTicks;
		}
		return result;
#endif
	}

}
