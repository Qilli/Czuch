#include "czpch.h"
#include "NativeFree3DCameraController.h"
#include "./Events/EventsTypes/InputEvents.h"
#include "./Events/EventsTypes/MouseEvents.h"
#include "Subsystems/Scenes/Components/CameraComponent.h"


namespace Czuch
{
	bool forwardButtonDown = false;
	bool backwardButtonDown = false;
	bool leftButtonDown = false;
	bool rightButtonDown = false;
	bool leftMouseButtonPressed = false;
	bool rightMouseButtonPressed = false;
	float mouseOffsetX = 0.0f;
	float mouseOffsetY = 0.0f;
	float lastMouseX = 0.0f;
	float lastMouseY = 0.0f;


	NativeFree3DCameraController::NativeFree3DCameraController(Entity entity)
	{
		SetEntity(entity);
		m_CameraTransformComponent = nullptr;
	}

	NativeFree3DCameraController::~NativeFree3DCameraController()
	{
	}

	void NativeFree3DCameraController::OnUpdate(TimeDelta delta)
	{
		if (forwardButtonDown)
		{
			m_CameraTransformComponent->Translate({ 0.0f,0.0f,-m_Speed * delta.GetDeltaTime() });
		}

		if (backwardButtonDown)
		{
			m_CameraTransformComponent->Translate({ 0.0f,0.0f,m_Speed * delta.GetDeltaTime() });
		}

		if (leftButtonDown)
		{
			m_CameraTransformComponent->Translate({ -m_Speed * delta.GetDeltaTime(),0.0f,0.0f });
		}

		if (rightButtonDown)
		{
			m_CameraTransformComponent->Translate({ m_Speed * delta.GetDeltaTime(),0.0f,0.0f });
		}

		if (leftMouseButtonPressed)
		{
			Vec3 eulerCurrent = m_CameraTransformComponent->GetLocalEulerAngles();

			eulerCurrent.x -= mouseOffsetY * m_Sensitivity;
			eulerCurrent.y -= mouseOffsetX * m_Sensitivity;

			if (eulerCurrent.x > 89.0f)
			{
				eulerCurrent.x = 89.0f;
			}
			if (eulerCurrent.x < -89.0f)
			{
				eulerCurrent.x = -89.0f;
			}
			m_CameraTransformComponent->SetLocalEulerAngles(eulerCurrent);

			mouseOffsetX = 0.0f;
			mouseOffsetY = 0.0f;
		}
	}

	void NativeFree3DCameraController::OnCreate()
	{
		auto scene = GetEntity().GetScene();
		auto camComponent = scene->FindPrimaryCamera();
		m_CameraTransformComponent=&camComponent->GetEntity().Transform();

	}

	void NativeFree3DCameraController::OnEnable()
	{
		LISTEN_TO_EVENT(KeyDownEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		LISTEN_TO_EVENT(KeyUpEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		LISTEN_TO_EVENT(MouseMovedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		LISTEN_TO_EVENT(MouseScrolledEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		LISTEN_TO_EVENT(MouseButtonDownEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		LISTEN_TO_EVENT(MouseButtonUpEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
	}

	void NativeFree3DCameraController::OnDisable()
	{
		STOP_LISTEN_TO_EVENT(KeyDownEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		STOP_LISTEN_TO_EVENT(KeyUpEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		STOP_LISTEN_TO_EVENT(MouseMovedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		STOP_LISTEN_TO_EVENT(MouseScrolledEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		STOP_LISTEN_TO_EVENT(MouseButtonDownEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		STOP_LISTEN_TO_EVENT(MouseButtonUpEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);	
	}

	void NativeFree3DCameraController::OnEvent(Event& event)
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
		}
		else if (event.GetEventType() == MouseMovedEvent::GetStaticEventType())
		{
			auto mouseMovedEvent = static_cast<const MouseMovedEvent&>(event);
			auto x = mouseMovedEvent.GetMouseX();
			auto y = mouseMovedEvent.GetMouseY();

			if (leftMouseButtonPressed)
			{
				mouseOffsetX = x -lastMouseX;
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
				leftMouseButtonPressed = true;
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
				leftMouseButtonPressed = false;
			}
			else if (mouseCode == Czuch::Mouse::ButtonRight)
			{
				rightMouseButtonPressed = false;
			}
		}
	}
}
