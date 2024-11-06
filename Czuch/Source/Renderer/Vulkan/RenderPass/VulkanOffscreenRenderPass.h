#pragma once
#include"Renderer/RenderPassControl.h"
#include"Renderer/Vulkan/VulkanCore.h"
#include"Renderer/Vulkan/VulkanDevice.h"

namespace Czuch
{
	class VulkanRenderer;
	class CommandBuffer;
	class Camera;
	class VulkanOffscreenRenderPass : public RenderPassControl
	{
	public:
		VulkanOffscreenRenderPass(Camera* cam, VulkanRenderer* renderer, VulkanDevice* device,
			U32 width, U32 height, bool handleWindowResize);
		~VulkanOffscreenRenderPass();
		void PreDraw(CommandBuffer* cmdBuffer,Renderer* renderer) override;
		void PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer) override;
		void Execute(CommandBuffer* cmdBuffer) override;
		void Resize(int width, int height) override;
		void* GetRenderPassResult() override;
	private:
		void Init();
		void Release();
	private:
		VulkanRenderer* m_Renderer;
		RenderPassHandle m_RenderPassHandle;
		VkDescriptorSet m_Descriptor;
		VulkanDevice* m_Device;
	};
}