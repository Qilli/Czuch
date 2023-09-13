#pragma once

namespace Czuch
{
	class Event;

	class IEventsListener
	{
	public:
		virtual void OnEvent(const Event& e)=0;
	};

}

