#pragma once
#include"BaseSubsystem.h"
#include"./Events/IEventsListener.h"
#include"./Events/Event.h"

namespace Czuch
{
	class CZUCH_API InputManager : public BaseSubsystem<InputManager>, public IEventsListener
	{
	public:
		void Init(RenderSettings* settings) override;
		void Shutdown() override;
		void Update(TimeDelta timeDelta) override;

		void OnEvent(Event& event) override;

		static bool IsKeyPressed(int keycode);
		static bool IsMouseButtonPressed(int button);
	private:
		static int m_Keys[512];
	private:
		static bool forwardButtonDown;
		static bool backwardButtonDown;
		static bool leftButtonDown;
		static bool rightButtonDown;
		static bool middleMouseButtonPressed;
		static bool rightMouseButtonPressed;
		static float mouseOffsetX;
		static float mouseOffsetY;
		static float lastMouseX;
		static float lastMouseY;
	};
}


