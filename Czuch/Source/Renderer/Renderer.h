#pragma once
#include"EngineRoot.h"

namespace Czuch
{
	enum class RendererAPI
	{
		OpenGL,
		Vulkan
	};

	class CZUCH_API Renderer
	{
	public:
		static RendererAPI GetUsedAPI(){ return RendererAPI::OpenGL; }
	};
}
