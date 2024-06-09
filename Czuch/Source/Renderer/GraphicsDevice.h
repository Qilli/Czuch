#pragma once
#include"Graphics.h"

namespace Czuch
{
	class CommandBuffer;
	struct RenderSettings;

	class GraphicsDevice
	{
	protected:
		GraphicsDeviceCapability m_DeviceCapabilities = GraphicsDeviceCapability::NONE;
		RenderSettings *m_RenderSettings;
	public:
		virtual ~GraphicsDevice() = default;
		virtual bool InitDevice(RenderSettings* settings) = 0;

		virtual float GetSwapchainWidth() const = 0;
		virtual float GetSwapchainHeight() const = 0;

		virtual void DrawUI(CommandBuffer* commandBuffer) = 0;

		virtual PipelineHandle CreatePipelineState(PipelineStateDesc* desc, const RenderPassHandle rpass, bool dynamicRendering = false) = 0;
		virtual ShaderHandle CreateShader(ShaderStage shaderStage, const char* shaderCode, size_t shaderCodeSize) =0;
		virtual RenderPassHandle CreateRenderPass(const RenderPassDesc* desc) = 0; 
		virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutDesc* desc) = 0;
		virtual FrameBufferHandle CreateFrameBuffer(const FrameBufferDesc* desc)  = 0;
		virtual CommandBufferHandle CreateCommandBuffer(bool isPrimary,void* pool=nullptr)  = 0;
		virtual BufferHandle CreateBuffer(const BufferDesc* desc) = 0;
		virtual TextureHandle CreateTexture(const TextureDesc* desc)= 0;
		virtual MeshHandle CreateMesh(MeshData& meshData) = 0;
		virtual MaterialHandle CreateMaterial(MaterialDesc& materialData) = 0;
		virtual MaterialInstanceHandle CreateMaterialInstance(MaterialInstanceDesc& materialInstanceDesc) = 0;

		virtual bool Release(PipelineHandle& pipeline) = 0;
		virtual bool Release(ShaderHandle& shader)= 0;
		virtual bool Release(RenderPassHandle& rp)= 0;
		virtual bool Release(DescriptorSetLayoutHandle& dsl) = 0;
		virtual bool Release(FrameBufferHandle& fb) = 0;
		virtual bool Release(CommandBufferHandle& commandBuffer)= 0;
		virtual bool Release(BufferHandle& buffer) = 0;
		virtual bool Release(TextureHandle& color_texture)= 0;
		virtual bool Release(MeshHandle& mesh) = 0;
		virtual bool Release(MaterialHandle& material) = 0;
		virtual bool Release(MaterialInstanceHandle& materialInstance) = 0;

		virtual Pipeline* AccessPipeline(PipelineHandle handle)= 0;
		virtual RenderPass* AccessRenderPass(RenderPassHandle handle) = 0;
		virtual Shader* AccessShader(ShaderHandle handle)= 0;
		virtual DescriptorSetLayout* AccessDescriptorSetLayout(DescriptorSetLayoutHandle handle) = 0;
		virtual FrameBuffer* AccessFrameBuffer(FrameBufferHandle handle)= 0;
		virtual CommandBuffer* AccessCommandBuffer(CommandBufferHandle handle)= 0;
		virtual Buffer* AccessBuffer(BufferHandle handle)= 0;
		virtual Texture* AccessTexture(TextureHandle handle)= 0;
		virtual Mesh* AccessMesh(MeshHandle handle) = 0;
		virtual Material* AccessMaterial(MaterialHandle handle) = 0;
		virtual MaterialInstance* AccessMaterialInstance(MaterialInstanceHandle handle) = 0;

	};

}
