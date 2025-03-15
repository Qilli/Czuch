#include"czpch.h"
#include "Event.h"

namespace Czuch
{
	void Event::Release(Event* evt)
	{
		delete evt;
	}
}

