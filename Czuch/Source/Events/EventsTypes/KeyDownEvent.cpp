#include"czpch.h"
#include "KeyDownEvent.h"
#include "Subsystems/EventsManager.h"

namespace Czuch
{

	void Czuch::KeyDownEvent::CreateAndDispatch(int keyCode)
	{
		KeyDownEvent* evt = new KeyDownEvent();
		evt->SetData(keyCode);
		EventsManager::GetPtr()->DispatchEvent(evt);
	}

}
