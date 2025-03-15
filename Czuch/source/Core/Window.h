#pragma once
#include"EngineCore.h"

namespace Czuch
{
	struct WindowParams
	{
		CzuchStr Title;
		U32 Width;
		U32 Height;

		WindowParams(const CzuchStr& title = "Czuch Engine", U32 width = 1000, U32 height = 333):
			Title(title),Width(width),Height(height)
		{

		}
	};

	class CZUCH_API Window
	{
	public:
		virtual ~Window() = default;
		virtual void Update() = 0;
		virtual U32 GetWidth() const = 0;
		virtual U32 GetHeight() const = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVsSync() const = 0;
		virtual void* GetNativeWindowPtr() const = 0;

		static Scope<Window> Create(const WindowParams& params = WindowParams());
	};

}