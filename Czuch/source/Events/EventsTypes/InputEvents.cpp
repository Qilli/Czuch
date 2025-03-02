#include"czpch.h"
#include"InputEvents.h"
#include "Subsystems/EventsManager.h"

namespace Czuch
{
	void KeyDownEvent::CreateAndDispatch(KeyCode keyCode,bool repeated)
	{
		KeyDownEvent* evt = new KeyDownEvent(keyCode,repeated);
		DISPATCH_EVENT(evt);
	}
	void KeyUpEvent::CreateAndDispatch(KeyCode keyCode)
	{
		KeyUpEvent* evt = new KeyUpEvent(keyCode);
		DISPATCH_EVENT(evt);
	}

	void KeyTypedEvent::CreateAndDispatch(KeyCode keyCode)
	{
		KeyTypedEvent* evt = new KeyTypedEvent(keyCode);
		DISPATCH_EVENT(evt);
	}

}