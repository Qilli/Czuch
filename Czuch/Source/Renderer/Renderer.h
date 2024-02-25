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

	struct SceneData
	{
		Mat4 view;
		Mat4 proj;
		Mat4 viewproj;
		Vec4 ambientColor;
	};

	class GraphicsDevice;

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
