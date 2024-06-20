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
	enum class RenderPassType;

	class CZUCH_API Renderer : public Czuch::IEventsListener
	{
	public:
		static RendererAPI GetUsedAPI() { return RendererAPI::Vulkan; }
	public:
		virtual ~Renderer() = default;
		virtual void DrawFrame() = 0;
		virtual void Init() = 0;
		virtual void AwaitDeviceIdle() = 0;
		virtual bool RegisterRenderContext(RenderContext* context) = 0;
		virtual void UnRegisterRenderContext(RenderContext* context) = 0;
		virtual void SetActiveScene(Scene* scene) = 0;
		virtual GraphicsDevice* GetDevice() = 0;
		virtual void AddRenderPass(RenderPassControl* renderPass) = 0;
		virtual void RemoveRenderPass(RenderPassType renderPassType) = 0;
		virtual void AddOffscreenRenderPass(Camera* cam, U32 width, U32 height, bool handleWindowResize, std::function<void(U32, U32)>* onResize) = 0;
		const RenderSettings& GetRenderSettings() const { return *m_RenderSettings; }
		virtual void* GetRenderPassResult(RenderPassType type) = 0;
		virtual bool HasRenderPass(RenderPassType type) = 0;
	protected:
		void OnEvent(Event& e) override;
		virtual void OnWindowResize(uint32_t width, uint32_t height) = 0;
	protected:
		RenderSettings* m_RenderSettings;
	};
}
