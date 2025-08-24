#pragma once
#include"GraphicsDevice.h"
#include"RenderContext.h"

namespace Czuch
{
	struct DescriptorSet;
	class DescriptorAllocator;
	struct Pipeline;
	class CommandBuffer
	{
	public:
		virtual void Init(const GraphicsDevice* gpu)=0;
		virtual void Release()=0;
	public:
		virtual void Begin(CommandBufferUseFlag flags = CommandBufferUseFlag::NONE)=0;
		virtual void End()=0;
		virtual void EndCurrentRenderPass()=0;
		virtual void BindPass(RenderPassHandle renderpass, FrameBufferHandle framebuffer)=0;
		virtual void DrawMesh(const RenderObjectInstance& renderElement,DescriptorAllocator* allocator)=0;
		virtual void DrawFullScreenQuad(MaterialInstanceHandle material, DescriptorAllocator* allocator) = 0;
		virtual void BindPipeline(PipelineHandle pipeline)=0;
		virtual void BindBuffer(BufferHandle buffer, U32 binding, U32 offset,BufferHandle *prevBuffer)=0;
		virtual Pipeline* BindMaterialInstance(MaterialInstanceHandle material, U32 passIndex, DescriptorAllocator* allocator) = 0;
		virtual void BindIndexBuffer(BufferHandle buffer, U32 offset)=0;
		virtual void BindDescriptorSet(DescriptorSet* descriptor,U16 setIndex, U32 num, U32* offsets, U32 num_offsets)=0;
		virtual void BindDescriptorSet(ParamSetUpdateControl* control, U16 setIndex, U32 num, U32* offsets, U32 num_offsets) = 0;
		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void SetDepthStencil(float depth, U8 stencil) = 0;
		virtual void SetViewport(ViewportDesc viewport) = 0;
		virtual void SetScrissors(ScissorsDesc scissors) = 0;
		virtual void Draw(U32 vertexCount, U32 firstVertex, U32 instanceCount, U32 firstInstance) = 0;
		virtual void DrawIndexed(U32 indicesCount, U32 firstIndex = 0, U32 instanceCount = 1, U32 firstnstance = 0, U32 vertexOffset = 0) = 0;
		virtual void DrawIndirectIndexedWithData(IndirectDrawForCommandBufferData* data,DescriptorAllocator* allocator) = 0;
	};

}
