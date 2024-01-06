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

		virtual Pipeline* CreatePipelineState(const PipelineStateDesc* desc, const RenderPass* rpass) const = 0;
		virtual Shader* CreateShader(ShaderStage shaderStage, const char* shaderCode, size_t shaderCodeSize) const=0;
		virtual RenderPass* CreateRenderPass(const RenderPassDesc* desc) const = 0; 
		virtual DescriptorSetLayout* CreateDescriptorSetLayout(const DescriptorSetLayoutDesc* desc) const = 0;
		virtual FrameBuffer* CreateFrameBuffer(const FrameBufferDesc* desc) const = 0;
		virtual CommandBuffer* CreateCommandBuffer(bool isPrimary) const = 0;
		virtual Buffer* CreateBuffer(const BufferDesc* desc) const = 0;

		virtual bool ReleasePipeline(Pipeline* pipeline) const = 0;
		virtual bool ReleaseShader(Shader* shader) const = 0;
		virtual bool ReleaseRenderPass(RenderPass* rp) const = 0;
		virtual bool ReleaseDescriptorSetLayout(DescriptorSetLayout* dsl) const = 0;
		virtual bool ReleaseFrameBuffer(FrameBuffer* fb) const = 0;
		virtual bool ReleaseCommandBuffer(CommandBuffer* commandBuffer)const = 0;
		virtual bool ReleaseBuffer(Buffer* buffer) const = 0;
	};

}
