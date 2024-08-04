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
		VulkanOffscreenRenderPass(RenderSettings* settings,Camera* cam,VulkanRenderer* renderer, VulkanDevice* device, Format colorFormat, Format depthFormat, U32 width, U32 height, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, bool handleWindowResize=false);
		~VulkanOffscreenRenderPass();
		void BeginRenderPass(CommandBuffer* cmdBuffer) override;
		void EndRenderPass(CommandBuffer* cmdBuffer) override;
		void Execute(CommandBuffer* cmdBuffer) override;
		void Resize(int width, int height) override;
		void* GetRenderPassResult() override;
	private:
		void Init();
		void Release();
		void DrawSceneToOffscreenBuffer(VulkanCommandBuffer* cmdBuffer);
	private:
		VulkanRenderer* m_Renderer;
		FrameBufferHandle m_FrameBuffer;
		TextureHandle m_Color, m_Depth;
		Texture_Vulkan* m_ColorTexture, * m_DepthTexture;
		VkRenderPass m_RenderPass;
		RenderPassHandle m_RenderPassHandle;
		VkDescriptorSet m_Descriptor;
		Format m_ColorFormat, m_DepthFormat;
		VulkanDevice* m_Device;
	};
}