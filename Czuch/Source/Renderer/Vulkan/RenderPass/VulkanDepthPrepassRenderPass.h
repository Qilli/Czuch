#pragma once
#include"Renderer/RenderPassControl.h"
#include"Renderer/Vulkan/VulkanDevice.h"

namespace Czuch
{
	class VulkanRenderer;
	class VulkanDepthPrepassRenderPass : public RenderPassControl
	{
	public:
		VulkanDepthPrepassRenderPass(VulkanRenderer* renderer, VulkanDevice* device,U32 width, U32 height, bool handleWindowResize);
		~VulkanDepthPrepassRenderPass();
		void PreDraw(CommandBuffer* cmdBuffer, Renderer* renderer) override;
		void PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer) override;
		void Execute(CommandBuffer* cmdBuffer) override;
		void Resize(int width, int height) override;
	private:
		void Init();
		void Release();
	private:
		VulkanRenderer* m_Renderer;
		VulkanDevice* m_Device;
		RenderContextFillParams m_FillParams;
	};
}
