#include"czpch.h"
#include "TimeDiffCounter.h"
#include "Time.h"

namespace Czuch
{

	TimeDiffCounter::TimeDiffCounter(Mode mode):ticks(),selectedMode(mode)
	{
	   LARGE_INTEGER freq;
	   Time::ReadHiResFrequency(freq);
	   frequency = F64(freq.QuadPart);
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
		ticks = U64(ticksCurrent.QuadPart);
	}

	F32 TimeDiffCounter::GetCounter(bool reset)
	{
		LARGE_INTEGER finalTicks;
		Time::ReadHiResCounter(finalTicks);

		F32 result=(F32)((finalTicks.QuadPart - ticks) / frequency);
		if (reset)
		{
			ticks = finalTicks.QuadPart;
		}
		return result;
	}

}
