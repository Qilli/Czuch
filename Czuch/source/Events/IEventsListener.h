#pragma once

namespace Czuch
{
	class Event;

	class IEventsListener
	{
	public:
		virtual void OnEvent(Event& e)=0;
	};

}

