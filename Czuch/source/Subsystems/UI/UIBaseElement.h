#pragma once
#include"Core/EngineCore.h"
#include"Core/TimeDelta.h"

namespace Czuch
{
	class CZUCH_API UIBaseElement
	{
	public:
		virtual void UpdateUI(TimeDelta timeDelta) {}
		virtual void OnFinishFrame() {}
	};
}
