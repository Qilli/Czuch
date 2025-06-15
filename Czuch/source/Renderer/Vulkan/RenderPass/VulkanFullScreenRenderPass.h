#pragma once
#include"VulkanRenderPassControlBase.h"

namespace Czuch
{
	class VulkanRenderer;
	class Texture_Vulkan;
	class VulkanDevice;
	class CommandBuffer;
	class Camera;
	class VulkanFullScreenRenderPass : public VulkanRenderPassControlBase
	{
	public:
		VulkanFullScreenRenderPass(VulkanDevice* device,VulkanRenderer* renderer);
		virtual ~VulkanFullScreenRenderPass() = default;
		void PreDraw(CommandBuffer* cmdBuffer,Renderer* renderer) override;
		void PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer) override;
		void Execute(CommandBuffer* cmdBuffer) override;
		void SetSourceTexture(CommandBuffer* cmd,TextureHandle texture,TextureHandle depthTexture, RenderPassHandle renderPass);
		void SetTargetTexture(CommandBuffer* cmd, TextureHandle texture);
		void SetViewportAndScissor(Camera* camera);
		void Release() override;
	private:
		void ReleaseRenderPassAndFrameBuffer();
		void CreateRenderPassAndFrameBuffer(RenderPassHandle handle);
		void CreateMaterial();
		void ReleaseMaterial();
	private:
		Viewport m_Viewport;
		RenderPassHandle m_RenderPassHandle;
		FrameBufferHandle m_FramebufferHandle;
		TextureHandle m_SourceTexture;
		TextureHandle m_DepthTexture;
		TextureHandle m_TargetTexture;
		AssetHandle m_MaterialAsset;
		AssetHandle m_MaterialInstanceAsset;
		MaterialInstanceHandle m_MaterialInstanceHandle;
	};
}