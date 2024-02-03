#pragma once
#include"Graphics.h"

namespace Czuch
{
	class CommandBuffer;

	class GraphicsDevice
	{
	protected:
		ValidationMode m_RendererValidationMode = ValidationMode::Disabled;
		GraphicsDeviceCapability m_DeviceCapabilities = GraphicsDeviceCapability::NONE;
	public:
		virtual ~GraphicsDevice() = default;
		virtual bool InitDevice() = 0;

		virtual float GetSwapchainWidth() const = 0;
		virtual float GetSwapchainHeight() const = 0;

		virtual PipelineHandle CreatePipelineState(const PipelineStateDesc* desc, const RenderPassHandle rpass) = 0;
		virtual ShaderHandle CreateShader(ShaderStage shaderStage, const char* shaderCode, size_t shaderCodeSize) =0;
		virtual RenderPassHandle CreateRenderPass(const RenderPassDesc* desc) = 0; 
		virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutDesc* desc) = 0;
		virtual FrameBufferHandle CreateFrameBuffer(const FrameBufferDesc* desc)  = 0;
		virtual CommandBufferHandle CreateCommandBuffer(bool isPrimary)  = 0;
		virtual BufferHandle CreateBuffer(const BufferDesc* desc) = 0;
		virtual TextureHandle CreateTexture(const TextureDesc* desc)= 0;

		virtual bool Release(PipelineHandle& pipeline) = 0;
		virtual bool Release(ShaderHandle& shader)= 0;
		virtual bool Release(RenderPassHandle& rp)= 0;
		virtual bool Release(DescriptorSetLayoutHandle& dsl) = 0;
		virtual bool Release(FrameBufferHandle& fb) = 0;
		virtual bool Release(CommandBufferHandle& commandBuffer)= 0;
		virtual bool Release(BufferHandle& buffer) = 0;
		virtual bool Release(TextureHandle& texture)= 0;

		virtual Pipeline* AccessPipeline(PipelineHandle handle)= 0;
		virtual RenderPass* AccessRenderPass(RenderPassHandle handle) = 0;
		virtual Shader* AccessShader(ShaderHandle handle)= 0;
		virtual DescriptorSetLayout* AccessDescriptorSetLayout(DescriptorSetLayoutHandle handle) = 0;
		virtual FrameBuffer* AccessFrameBuffer(FrameBufferHandle handle)= 0;
		virtual CommandBuffer* AccessCommandBuffer(CommandBufferHandle handle)= 0;
		virtual Buffer* AccessBuffer(BufferHandle handle)= 0;
		virtual Texture* AccessTexture(TextureHandle handle)= 0;

	};

}
