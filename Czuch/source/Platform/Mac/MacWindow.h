#pragma once

#include "Core/Window.h"
#include "GLFW/glfw3.h"

namespace Czuch
{
    class MacWindow : public Window
    {
    public:
        MacWindow(const WindowParams& params);
        virtual ~MacWindow();

        void Update() override;

        unsigned int GetWidth() const override { return m_Data.Width; }
        unsigned int GetHeight() const override { return m_Data.Height; }

        // Window attributes
        void SetVSync(bool enabled) override;
        bool IsVsSync() const override;

        // Crucial for Vulkan surface creation
        virtual void* GetNativeWindowPtr() const override { return m_Window; }

    private:
        virtual void Init(const WindowParams& params);
        virtual void Shutdown();

    private:
        GLFWwindow* m_Window;

        // Struct to hold window state, passed to GLFW callbacks via UserPointer
        struct WindowData
        {
            CzuchStr Title;
            unsigned int Width, Height;
            bool VSync;
        };

        WindowData m_Data;
    };
}