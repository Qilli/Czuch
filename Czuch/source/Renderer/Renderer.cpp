#include "czpch.h"
#include "Renderer.h"
#include"Events/EventsTypes/ApplicationEvents.h"

namespace Czuch
{
	void Czuch::Renderer::OnEvent(const Event& e)
	{
		auto windowSizeChangedEvent = dynamic_cast<const WindowSizeChangedEvent*>(&e);
		if (windowSizeChangedEvent)
		{
			OnWindowResize(windowSizeChangedEvent->GetWidth(), windowSizeChangedEvent->GetHeight());
		}
	}
}

