#pragma once
#include"Renderer/RenderPassControl.h"
#include"Renderer/Vulkan/VulkanDevice.h"

namespace Czuch
{
	class VulkanRenderer;
	class VulkanMainRenderPass : public RenderPassControl
	{
	public:
		VulkanMainRenderPass(VulkanRenderer* renderer, VulkanDevice* device);
		~VulkanMainRenderPass();
		void BeginRenderPass(CommandBuffer* cmdBuffer) override;
		void EndRenderPass(CommandBuffer* cmdBuffer) override;
		void Execute(CommandBuffer* cmdBuffer) override;
		void Resize(int width, int height) override;
	private:
		void Init();
		void Release();
	private:
		VulkanRenderer* m_Renderer;
		VulkanDevice* m_Device;
	};
}