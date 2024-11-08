#pragma once
#include"Renderer/RenderPassControl.h"
#include"Renderer/Vulkan/VulkanDevice.h"

namespace Czuch
{
	class VulkanRenderer;
	class Texture_Vulkan;
	class VulkanMainRenderPass : public RenderPassControl
	{
	public:
		VulkanMainRenderPass(VulkanDevice* device);
		~VulkanMainRenderPass();
		void PreDraw(CommandBuffer* cmdBuffer,Renderer* renderer) override;
		void PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer) override;
		void Execute(CommandBuffer* cmdBuffer) override;
		void Resize(int width, int height) override;
		void SetFinalTexture(Texture_Vulkan* texture) { m_FinalTexture = texture; }
	private:
		void Init();
		void Release();
	private:
		VulkanDevice* m_Device;
		Texture_Vulkan* m_FinalTexture;
	};
}