#include "czpch.h"
#include "MouseEvents.h"
#include"Subsystems/EventsManager.h"

namespace Czuch
{
	void MouseButtonDownEvent::CreateAndDispatch(MouseCode mouseCode)
	{
		MouseButtonDownEvent* evt = new MouseButtonDownEvent(mouseCode);
		DISPATCH_EVENT(evt);
	}
	void MouseButtonUpEvent::CreateAndDispatch(MouseCode mouseCode)
	{
		MouseButtonUpEvent* evt = new MouseButtonUpEvent(mouseCode);
		DISPATCH_EVENT(evt);
	}
	void MouseMovedEvent::CreateAndDispatch(const F32 x, const F32 y)
	{
		MouseMovedEvent* evt = new MouseMovedEvent(x,y);
		DISPATCH_EVENT(evt);
	}
	void MouseScrolledEvent::CreateAndDispatch(const F32 xOffset, const F32 yOffset)
	{
		MouseScrolledEvent* evt = new MouseScrolledEvent(xOffset, yOffset);
		DISPATCH_EVENT(evt);
	}
}
