#include "czpch.h"
#include "MacWindow.h"

#include "Subsystems/Logging.h"
#include "Renderer/Renderer.h"
#include "Events/EventsTypes/ApplicationEvents.h"
#include "Events/EventsTypes/InputEvents.h"
#include "Events/EventsTypes/MouseEvents.h"

namespace Czuch
{
    static uint8_t s_GLFWWindowCount = 0;

    static void GLFWErrorCallback(int error, const char *desc)
    {
        LOG_BE_ERROR("GLFW Error ({0}): {1}", error, desc);
    }

    MacWindow::MacWindow(const WindowParams &params)
    {
        Init(params);
    }

    MacWindow::~MacWindow()
    {
        Shutdown();
    }

    void MacWindow::Init(const WindowParams &params)
    {
        m_Data.Title = params.Title;
        m_Data.Width = params.Width;
        m_Data.Height = params.Height;

        LOG_BE_INFO("Creating macOS window {0} ({1}, {2})", params.Title, params.Width, params.Height);

        if (s_GLFWWindowCount == 0)
        {
            int success = glfwInit();
            CZUCH_BE_ASSERT(success, "Failed to init GLFW");
            glfwSetErrorCallback(GLFWErrorCallback);
        }

        // --- MacOS / Vulkan Specific Hints ---

        // If using Vulkan, we strictly need NO_API.
        // MacOS uses MoltenVK which runs over Metal, but GLFW handles this if configured correctly.
        if (Renderer::GetUsedAPI() == RendererAPI::Vulkan)
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

#ifdef CZUCH_DEBUG
        if (Renderer::GetUsedAPI() == RendererAPI::OpenGL)
        {
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        }
#endif

        // Provide a hint for Retina displays (High DPI)
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);

        m_Window = glfwCreateWindow((int)params.Width, (int)params.Height, m_Data.Title.c_str(), nullptr, nullptr);
        ++s_GLFWWindowCount;

        // Create Context (Only if NOT Vulkan)
        if (Renderer::GetUsedAPI() != RendererAPI::Vulkan)
        {
            glfwMakeContextCurrent(m_Window);
        }

        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(true);

        // --- Set GLFW Callbacks ---

        // SIZE: On macOS, we care about Framebuffer size (pixels) vs Window Size (screen points)
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow *window, int width, int height)
                                  {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;

            WindowSizeChangedEvent::CreateAndDispatch(width, height);
         });

        // Also listen to Framebuffer resize (Retina handling)
        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow *window, int width, int height)
                                       {
             WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
             data.Width = width;
             data.Height = height;
             
             // Dispatch with actual pixel dimensions
             WindowSizeChangedEvent::CreateAndDispatch(width, height);
             });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window)
                                   { WindowClosedEvent::CreateAndDispatch(); });

        glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
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
            } });

        glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int keycode)
                            { KeyTypedEvent::CreateAndDispatch(keycode); });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int mods)
                                   {
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
            } });

        glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double xOffset, double yOffset)
                              { MouseScrolledEvent::CreateAndDispatch((float)xOffset, (float)yOffset); });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double xPos, double yPos)
                                 { MouseMovedEvent::CreateAndDispatch((float)xPos, (float)yPos); });
    }

    void MacWindow::Shutdown()
    {
        glfwDestroyWindow(m_Window);
        --s_GLFWWindowCount;

        if (s_GLFWWindowCount == 0)
        {
            glfwTerminate();
        }
    }

    void MacWindow::Update()
    {
        // On MacOS, polling events must happen on the main thread.
        glfwPollEvents();

        // Only swap buffers if NOT Vulkan (Vulkan handles its own swapchain)
        if (Renderer::GetUsedAPI() != RendererAPI::Vulkan)
        {
            glfwSwapBuffers(m_Window);
        }
    }

    void MacWindow::SetVSync(bool enabled)
    {
        m_Data.VSync = enabled;

        if (Renderer::GetUsedAPI() != RendererAPI::Vulkan)
        {
            if (enabled)
                glfwSwapInterval(1);
            else
                glfwSwapInterval(0);
        }
        // Note: Vulkan VSync is handled during SwapChain creation (Mailbox vs FIFO present mode), not here.
    }

    bool MacWindow::IsVsSync() const
    {
        return m_Data.VSync;
    }
}