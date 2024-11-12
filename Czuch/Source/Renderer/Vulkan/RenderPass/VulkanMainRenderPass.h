#pragma once
#include"VulkanRenderPassControlBase.h"

namespace Czuch
{
	class VulkanRenderer;
	class Texture_Vulkan;
	class VulkanDevice;
	class CommandBuffer;
	class Camera;
	class VulkanMainRenderPass : public VulkanRenderPassControlBase
	{
	public:
		VulkanMainRenderPass(VulkanDevice* device,VulkanRenderer* renderer);
		virtual ~VulkanMainRenderPass() = default;
		void PreDraw(CommandBuffer* cmdBuffer,Renderer* renderer) override;
		void PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer) override;
		void Execute(CommandBuffer* cmdBuffer) override;
		void Resize(int width, int height) override;
		void SetFinalTexture(Texture_Vulkan* texture);
	private:
		Texture_Vulkan* m_FinalTexture;
	};
}