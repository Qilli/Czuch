#include"czpch.h"
#include"Window.h"
#include"Platform/Windows/WinWindow.h"

namespace Czuch
{
	Scope<Window> Window::Create(const WindowParams& params)
	{
#ifdef CZUCH_PLATFORM_WINDOWS
		return CreateScoped<WinWindow>(params);
#else
		CZUCH_BE_ASSERT(false, "Unsupported platform!");
		return nullptr;
#endif
	}
}