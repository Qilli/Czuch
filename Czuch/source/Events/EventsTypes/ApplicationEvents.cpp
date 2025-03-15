#include"czpch.h"
#include "ApplicationEvents.h"
#include "Subsystems/EventsManager.h"

namespace Czuch
{

	void WindowSizeChangedEvent::CreateAndDispatch(U32 width, U32 height)
	{
		WindowSizeChangedEvent* evt = new WindowSizeChangedEvent();
		evt->SetData(width, height);
		DISPATCH_EVENT(evt);
	}

	void WindowClosedEvent::CreateAndDispatch()
	{
		WindowClosedEvent* evt = new WindowClosedEvent();
		DISPATCH_EVENT(evt);
	}

}
