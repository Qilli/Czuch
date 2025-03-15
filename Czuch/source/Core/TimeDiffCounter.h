#pragma once
#include "EngineCore.h"

namespace Czuch
{
	class CZUCH_API TimeDiffCounter
	{
	public:
		enum class Mode
		{
			Seconds,
			Miliseconds,
			Microseconds
		};
	public:
		TimeDiffCounter(Mode mode);
		TimeDiffCounter() = delete;
		TimeDiffCounter(const TimeDiffCounter& c) = delete;
		void StartCounter();
		F32 GetCounter(bool reset=true);
	private:
		void ApplyMode();
	private:
		F64 frequency;
		long long ticks;
		Mode selectedMode;
	};
}