#pragma once

#include"Core/Window.h"
#include"GLFW/glfw3.h"

namespace Czuch
{
	class WinWindow : public Window
	{
	public:
		WinWindow(const WindowParams& params);
		virtual ~WinWindow();

		virtual void Update() override;
		virtual U32 GetWidth() const override;
		virtual U32 GetHeight() const override;
		virtual void SetVSync(bool enabled) override;
		virtual bool IsVsSync() const override;
		virtual void* GetNativeWindowPtr() const { return m_Window; }
	private:
		virtual void Init(const WindowParams& params);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;

		struct WinParamsData
		{
			CzuchStr title;
			U32 Width, Height;
			bool VSync;
		};

		WinParamsData m_WndParams;
	};
}
