#pragma once
#include"Renderer/CommandBuffer.h"
#include"Renderer/Vulkan/DescriptorAllocator.h"
namespace Czuch
{
	class VulkanDevice;
	class DescriptorAllocator;

	class VulkanCommandBuffer final : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(VkCommandBuffer cmdBuffer);
		void Init(const GraphicsDevice* gpu) override;
		void Release() override;
		const VkCommandBuffer GetNativeBuffer() const { return m_Cmd; }
	public:
		void Begin(CommandBufferUseFlag flags = CommandBufferUseFlag::NONE) override;
		void End() override;
		void EndCurrentRenderPass() override;
		void DrawMesh(const RenderObjectInstance& renderElement, DescriptorAllocator* allocator) override;
		void TryBindMeshInstanceBuffers(Czuch::Mesh* meshInstance);
		void DrawFullScreenQuad(MaterialInstanceHandle material, DescriptorAllocator* allocator) override;
		void BindPass(RenderPassHandle renderpass, FrameBufferHandle framebuffer) override;
		void BindPipeline(PipelineHandle pipeline) override;
		Pipeline* BindMaterialInstance(MaterialInstanceHandle material, U32 passIndex, DescriptorAllocator* allocator) override;
		void BindBuffer(BufferHandle buffer, U32 binding, U32 offset, BufferHandle *prevBuffer) override;
		void BindIndexBuffer(BufferHandle buffer, U32 offset) override;
		void BindDescriptorSet(DescriptorSet* descriptor,U16 setIndex, U32 num, U32* offsets, U32 num_offsets) override;
		void BindDescriptorSet(ParamSetUpdateControl* control, U16 setIndex, U32 num, U32* offsets, U32 num_offsets) override;
		void SetClearColor(float r, float g, float b, float a) override;
		void SetDepthStencil(float depth, U8 stencil) override;
		void SetViewport(ViewportDesc viewport) override;
		void SetScrissors(ScissorsDesc scissors) override;
		void Draw(U32 vertexCount, U32 firstVertex=0, U32 instanceCount=1, U32 firstInstance=0) override;
		void DrawIndexed(U32 indicesCount, U32 firstIndex=0, U32 instanceCount=1, U32 firstnstance=0, U32 vertexOffset=0) override;
		void DrawIndirectIndexedWithData(IndirectDrawForCommandBufferData* data,DescriptorAllocator* allocator) override;
	public:
		void BeginDynamicRenderPassForMainPass(VkImageView colorVew, VkImageView depthView,U32 width,U32 height);
		void EndDynamicRenderPassForMainPass();
	private:
		void DrawQuadInternal();
	private:
		VkCommandBuffer m_Cmd;
		VulkanDevice* m_Device;
		VkClearValue m_ClearValueColor;
		VkClearValue m_ClearValueDepth;
		bool m_isRecording;
		RenderPassHandle m_CurrentRenderPass;
		FrameBufferHandle m_CurrentFrameBuffer;
		PipelineHandle m_CurrentPipeline;
		ViewportDesc m_CurrentViewport;
		ScissorsDesc m_CurrentScissors;
		Array<VkRenderingAttachmentInfoKHR> m_ColorAttachmentsInfo;
	private:
		BufferHandle m_CurrentIndexBuffer;
		BufferHandle m_CurrentVertexBuffer;
		MaterialInstanceHandle m_CurrentMaterialHandle;
		DescriptorWriter m_Writer;
	};

}

