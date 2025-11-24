#include"czpch.h"
#include"Window.h"
#include"Platform/Windows/WinWindow.h"
#include"Platform/Mac/MacWindow.h"

namespace Czuch
{
	Scope<Window> Window::Create(const WindowParams& params)
	{
#ifdef CZUCH_PLATFORM_WINDOWS
		return CreateScoped<WinWindow>(params);
#elif CZUCH_PLATFORM_MACOS
		return CreateScoped<MacWindow>(params);
#else
		CZUCH_BE_ASSERT(false, "Unsupported platform!");
		return nullptr;
#endif
	}
}