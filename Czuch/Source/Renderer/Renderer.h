#pragma once
#include"Graphics.h"

namespace Czuch
{
	enum class RendererAPI
	{
		None,
		OpenGL,
		Vulkan
	};

	class CZUCH_API Renderer
	{
	public:
		static RendererAPI GetUsedAPI(){ return RendererAPI::Vulkan; }
	public:
		virtual ~Renderer() = default;
		virtual void DrawFrame() = 0;
		virtual void Init() = 0;
		virtual void AwaitDeviceIdle() = 0;
		virtual GraphicsDevice* GetDevice() = 0;
	};
}
