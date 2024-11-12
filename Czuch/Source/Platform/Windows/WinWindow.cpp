#include "czpch.h"
#include "WinWindow.h"
#include"Subsystems/Logging.h"
#include"Renderer/Renderer.h"
#include"Events/EventsTypes/ApplicationEvents.h"
#include"Events/EventsTypes/InputEvents.h"
#include"Events/EventsTypes/MouseEvents.h"

namespace Czuch
{
	static U8 s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const I8* desc)
	{
		LOG_BE_ERROR("GLFW internal error ({0}): {1}", error, desc);
	}

	WinWindow::WinWindow(const WindowParams& params)
	{
		Init(params);
	}

	WinWindow::~WinWindow()
	{
		Shutdown();
	}

	void WinWindow::Update()
	{
		glfwPollEvents();
	}

	U32 WinWindow::GetWidth() const
	{
		return m_WndParams.Width;
	}

	U32 WinWindow::GetHeight() const
	{
		return m_WndParams.Height;
	}

	void WinWindow::SetVSync(bool enabled)
	{
		m_WndParams.VSync = enabled;

		/*if (enabled)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}*/
	}

	bool WinWindow::IsVsSync() const
	{
		return m_WndParams.VSync;
	}

	void WinWindow::Init(const WindowParams& params)
	{
		m_WndParams.title = params.Title;
		m_WndParams.Width = params.Width;
		m_WndParams.Height = params.Height;

		LOG_BE_INFO("Creating window {0} ({1}, {2})", m_WndParams.title, m_WndParams.Width, m_WndParams.Height);

		if (s_GLFWWindowCount == 0)
		{
			int success = glfwInit();
			CZUCH_BE_ASSERT(success, "Failed to init GLFW");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

#ifdef CZUCH_DEBUG
		if (Renderer::GetUsedAPI() == RendererAPI::OpenGL)
		{
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		}
#endif
		if (Renderer::GetUsedAPI() == RendererAPI::Vulkan)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}

		m_Window = glfwCreateWindow(m_WndParams.Width, m_WndParams.Height, m_WndParams.title.c_str(), nullptr, nullptr);
		++s_GLFWWindowCount;

		if (Renderer::GetUsedAPI() != RendererAPI::Vulkan)
		{
			glfwMakeContextCurrent(m_Window);
		}
		glfwSetWindowUserPointer(m_Window, &m_WndParams);
		SetVSync(true);

		//callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* wnd, I32 width, I32 height)
			{
				WinParamsData data = *((WinParamsData*)glfwGetWindowUserPointer(wnd));
				data.Width = width;
				data.Height = height;
				WindowSizeChangedEvent::CreateAndDispatch(width, height);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* wnd) {
			WindowClosedEvent::CreateAndDispatch();
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* wnd, I32 key, I32 scancode, I32 action, I32 mods)
			{
				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyDownEvent::CreateAndDispatch(key, false);
						break;
					}
					case GLFW_RELEASE:
					{
						KeyUpEvent::CreateAndDispatch(key);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyDownEvent::CreateAndDispatch(key, true);
						break;
					}
				}
			});


		glfwSetCharCallback(m_Window, [](GLFWwindow* wmd, U32 keyCode) {
			KeyTypedEvent::CreateAndDispatch(keyCode);
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* wnd, int button, int action, int mods) {

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonDownEvent::CreateAndDispatch(button);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonUpEvent::CreateAndDispatch(button);
					break;
				}

			}
		
			});


		glfwSetScrollCallback(m_Window, [](GLFWwindow* wnd, double xOffset,double yOffset) {

			MouseScrolledEvent::CreateAndDispatch((float)xOffset, (float)yOffset);

			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* wnd, double x, double y) {
			MouseMovedEvent::CreateAndDispatch(x, y);
			});
	}

	void WinWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
		s_GLFWWindowCount--;

		if (s_GLFWWindowCount == 0)
		{
			glfwTerminate();
		}
	}
}
