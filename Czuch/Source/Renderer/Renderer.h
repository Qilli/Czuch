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

	class GraphicsDevice;
	struct RenderContextCreateInfo;
	class RenderContext;
	class Scene;

	class CZUCH_API Renderer
	{
	public:
		static RendererAPI GetUsedAPI(){ return RendererAPI::Vulkan; }
	public:
		virtual ~Renderer() = default;
		virtual void DrawFrame() = 0;
		virtual void Init() = 0;
		virtual void AwaitDeviceIdle() = 0;
		virtual bool RegisterRenderContext(RenderContext* context) = 0;
		virtual void UnRegisterRenderContext(RenderContext* context) = 0;
		virtual void SetActiveScene(Scene* scene) = 0;
		virtual GraphicsDevice* GetDevice() = 0;
	};
}
