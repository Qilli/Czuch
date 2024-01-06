#pragma once
#include"Renderer/CommandBuffer.h"
#include"VulkanBase.h"
namespace Czuch
{
	class VulkanDevice;

	class VulkanCommandBuffer final : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(VkCommandBuffer cmdBuffer);
		void Init(const GraphicsDevice* gpu) override;
		void Release() override;
		const VkCommandBuffer GetNativeBuffer() const { return m_Cmd; }
	public:
		void Begin() override;
		void End() override;
		void EndCurrentRenderPass() override;
		void BindPass(RenderPass* renderpass, FrameBuffer* framebuffer) override;
		void BindPipeline(Pipeline* pipeline) override;
		void BindVertexBuffer(Buffer* buffer, U32 binding, U32 offset) override;
		void BindIndexBuffer(Buffer* buffer, U32 offset) override;
		void BindDescriptorSet(DescriptorSet* descriptor, U32 num, U32* offsets, U32 num_offsets) override;
		void SetClearColor(float r, float g, float b, float a) override;
		void SetDepthStencil(float depth, U8 stencil) override;
		void SetViewport(ViewportDesc viewport) override;
		void SetScrissors(ScissorsDesc scissors) override;
		void Draw(U32 vertexCount, U32 firstVertex, U32 instanceCount, U32 firstInstance) override;
	private:
		VkCommandBuffer m_Cmd;
		VulkanDevice* m_Device;
		VkClearValue m_ClearValue;
		bool m_isRecording;
		RenderPass* m_CurrentRenderPass;
		FrameBuffer* m_CurrentFrameBuffer;
		Pipeline* m_CurrentPipeline;
		ViewportDesc m_CurrentViewport;
		ScissorsDesc m_CurrentScissors;
	};

}

