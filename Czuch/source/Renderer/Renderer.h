#pragma once
#include"Graphics.h"
#include"Events/IEventsListener.h"
#include<functional>

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
	class RenderPassControl;
	class Scene;
	class Camera;
	struct RenderContextFillParams;
	struct FrameGraph;

	class CZUCH_API Renderer : public IEventsListener
	{
	public:
		static RendererAPI GetUsedAPI() { return RendererAPI::Vulkan; }
	public:
		virtual ~Renderer() = default;
		virtual void DrawFrame() = 0;
		virtual void Init() = 0;
		virtual void CreateRenderGraph() = 0;
		virtual void AfterSystemInit() = 0;
		virtual void ReleaseDependencies() = 0;
		virtual void AwaitDeviceIdle() = 0;
		virtual void SetActiveScene(Scene* scene) = 0;
		virtual GraphicsDevice* GetDevice() = 0;
		virtual void RegisterRenderPassResizeEventResponse(U32 width, U32 height, bool handleWindowResize, std::function<void(U32, U32)>* onResize) = 0;
		virtual void* GetRenderPassResult(Camera* cam,RenderPassType type) = 0;
		virtual bool HasRenderPass(Camera* cam,RenderPassType type) = 0;
		virtual void OnPreRenderUpdateContexts(Camera* camera, int width, int height,RenderContextFillParams* fillParams)=0;
		virtual void OnPostRenderUpdateContexts(Camera* camera,RenderContextFillParams* fillParams)=0;
		virtual void OnPreRenderUpdateDebugDrawElements(Camera* camera, RenderContextFillParams* fillParams) = 0;
		virtual void* GetFrameGraphFinalResult(Camera* cam) = 0;
		virtual void FlushDeletionQueue() = 0;
	
	protected:
		void OnEvent(Event& e) override;
		virtual void OnWindowResize(uint32_t width, uint32_t height) = 0;
	};
}
