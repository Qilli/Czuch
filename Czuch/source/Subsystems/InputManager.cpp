#include "czpch.h"
#include "InputManager.h"
#include"Core/KeyCodes.h"
#include "./Events/EventsTypes/InputEvents.h"
#include "./Events/EventsTypes/MouseEvents.h"
#include"EventsManager.h"

namespace Czuch
{
	bool InputManager::forwardButtonDown = false;
	bool InputManager::backwardButtonDown = false;
	bool InputManager::leftButtonDown = false;
	bool InputManager::rightButtonDown = false;
	bool InputManager::middleMouseButtonPressed = false;
	bool InputManager::rightMouseButtonPressed = false;
	float InputManager::mouseOffsetX = 0.0f;
	float InputManager::mouseOffsetY = 0.0f;
	float InputManager::lastMouseX = 0.0f;
	float InputManager::lastMouseY = 0.0f;
	int InputManager::m_Keys[512];


	void InputManager::Shutdown()
	{
		STOP_LISTEN_TO_EVENT(KeyDownEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		STOP_LISTEN_TO_EVENT(KeyUpEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		STOP_LISTEN_TO_EVENT(MouseMovedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		STOP_LISTEN_TO_EVENT(MouseScrolledEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		STOP_LISTEN_TO_EVENT(MouseButtonDownEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		STOP_LISTEN_TO_EVENT(MouseButtonUpEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
	}

	void InputManager::Update(TimeDelta timeDelta)
	{
	}

	void InputManager::OnEvent(Event& event)
	{
	
		if (event.GetEventType() == KeyDownEvent::GetStaticEventType())
		{
			auto keyDownEvent = static_cast<const KeyDownEvent&>(event);
			auto keyCode = keyDownEvent.GetKeyCode();
			if (keyCode == Czuch::Key::W)
			{
				forwardButtonDown = true;
			}
			else if (keyCode == Czuch::Key::S)
			{
				backwardButtonDown = true;
			}
			else if (keyCode == Czuch::Key::A)
			{
				leftButtonDown = true;
			}
			else if (keyCode == Czuch::Key::D)
			{
				rightButtonDown = true;
			}

			m_Keys[keyCode] = 1;
		}
		else if (event.GetEventType() == KeyUpEvent::GetStaticEventType())
		{
			auto keyUpEvent = static_cast<const KeyUpEvent&>(event);
			auto keyCode = keyUpEvent.GetKeyCode();
			if (keyCode == Czuch::Key::W)
			{
				forwardButtonDown = false;
			}
			else if (keyCode == Czuch::Key::S)
			{
				backwardButtonDown = false;
			}
			else if (keyCode == Czuch::Key::A)
			{
				leftButtonDown = false;
			}
			else if (keyCode == Czuch::Key::D)
			{
				rightButtonDown = false;
			}

			m_Keys[keyCode] = 0;
		}
		else if (event.GetEventType() == MouseMovedEvent::GetStaticEventType())
		{
			auto mouseMovedEvent = static_cast<const MouseMovedEvent&>(event);
			auto x = mouseMovedEvent.GetMouseX();
			auto y = mouseMovedEvent.GetMouseY();

			if (middleMouseButtonPressed)
			{
				mouseOffsetX = x - lastMouseX;
				mouseOffsetY = y - lastMouseY;
				lastMouseX = x;
				lastMouseY = y;
			}
			else
			{
				lastMouseX = x;
				lastMouseY = y;
			}

		}
		else if (event.GetEventType() == MouseScrolledEvent::GetStaticEventType())
		{

		}
		else if (event.GetEventType() == MouseButtonDownEvent::GetStaticEventType())
		{
			auto mouseButtonDownEvent = static_cast<const MouseButtonDownEvent&>(event);
			auto mouseCode = mouseButtonDownEvent.GetMouseCode();

			if (mouseCode == Czuch::Mouse::ButtonLeft)
			{
				middleMouseButtonPressed = true;
			}
			else if (mouseCode == Czuch::Mouse::ButtonRight)
			{
				rightMouseButtonPressed = true;
			}
		}
		else if (event.GetEventType() == MouseButtonUpEvent::GetStaticEventType())
		{
			auto mouseButtonUpEvent = static_cast<const MouseButtonUpEvent&>(event);
			auto mouseCode = mouseButtonUpEvent.GetMouseCode();

			if (mouseCode == Czuch::Mouse::ButtonLeft)
			{
				middleMouseButtonPressed = false;
			}
			else if (mouseCode == Czuch::Mouse::ButtonRight)
			{
				rightMouseButtonPressed = false;
			}
		}
	}

	bool InputManager::IsKeyPressed(int keycode)
	{
		return m_Keys[keycode]==1;
	}

	bool InputManager::IsMouseButtonPressed(int button)
	{
		if (button == 0)
		{
			return middleMouseButtonPressed;
		}
		else if (button == 1)
		{
			return rightMouseButtonPressed;
		}
		return false;
	}

	void InputManager::Init(EngineSettings* settings)
	{
		BaseSubsystem::Init(settings);
		memset(m_Keys, 0, 512);

		LISTEN_TO_EVENT(KeyDownEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		LISTEN_TO_EVENT(KeyUpEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		LISTEN_TO_EVENT(MouseMovedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		LISTEN_TO_EVENT(MouseScrolledEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		LISTEN_TO_EVENT(MouseButtonDownEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		LISTEN_TO_EVENT(MouseButtonUpEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
	}
}

