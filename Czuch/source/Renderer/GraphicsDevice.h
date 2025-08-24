#pragma once
#include"Graphics.h"

namespace Czuch
{
	class CommandBuffer;
	struct EngineSettings;
	struct FrameGraphBuilderHelper;

	enum class RenderMode
	{
		Default,
		Offscreen
	};

	class GraphicsDevice
	{
	protected:
		GraphicsDeviceCapability m_DeviceCapabilities = GraphicsDeviceCapability::NONE;
		EngineSettings *m_RenderSettings;
		RenderMode m_RenderMode;
	public:
		virtual ~GraphicsDevice() = default;
		virtual bool InitDevice(EngineSettings* settings) = 0;

		virtual float GetSwapchainWidth() const = 0;
		virtual float GetSwapchainHeight() const = 0;

		virtual void DrawUI(CommandBuffer* commandBuffer) = 0;
		virtual void SetRenderMode(RenderMode mode) { m_RenderMode = mode; }
		virtual RenderMode GetRenderMode() const { return m_RenderMode; }
		virtual void* CreatePointerForUITexture(TextureHandle tex) = 0;

		virtual PipelineHandle CreatePipelineState(const MaterialPassDesc* desc,RenderPass* rpass, bool dynamicRendering = false) = 0;
		virtual ShaderHandle CreateShader(ShaderStage shaderStage, const char* shaderCode, size_t shaderCodeSize) =0;
		virtual RenderPassHandle CreateRenderPass(const RenderPassDesc* desc) = 0; 
		virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutDesc* desc) = 0;
		virtual FrameBufferHandle CreateFrameBuffer(const FrameBufferDesc* desc, bool resize = false, FrameBufferHandle handle = INVALID_HANDLE(FrameBufferHandle))  = 0;
		virtual CommandBufferHandle CreateCommandBuffer(bool isPrimary,void* pool=nullptr)  = 0;
		virtual BufferHandle CreateBuffer(const BufferDesc* desc) = 0;
		virtual TextureHandle CreateTexture(const TextureDesc* desc,bool resize=false,TextureHandle handle= INVALID_HANDLE(TextureHandle))= 0;
		virtual MeshHandle CreateMesh(MeshData& meshData) = 0;
		virtual MaterialHandle CreateMaterial(MaterialDefinitionDesc* materialData) = 0;
		virtual MaterialInstanceHandle CreateMaterialInstance(MaterialInstanceDesc& materialInstanceDesc) = 0;
		virtual BufferHandle CreateUBOBuffer(MaterialCustomBufferData* ubo) = 0;
		virtual BufferHandle CreateSSBOBuffer(U32 elementsCount, U32 elemSize,bool permaMapped=true) = 0;

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
		virtual RenderPass* GetRenderPassOfType(RenderPassType type) = 0;
		virtual Shader* AccessShader(ShaderHandle handle)= 0;
		virtual DescriptorSetLayout* AccessDescriptorSetLayout(DescriptorSetLayoutHandle handle) = 0;
		virtual FrameBuffer* AccessFrameBuffer(FrameBufferHandle handle)= 0;
		virtual CommandBuffer* AccessCommandBuffer(CommandBufferHandle handle)= 0;
		virtual Buffer* AccessBuffer(BufferHandle handle)= 0;
		virtual Texture* AccessTexture(TextureHandle handle)= 0;
		virtual Mesh* AccessMesh(MeshHandle handle) = 0;
		virtual Material* AccessMaterial(MaterialHandle handle) = 0;
		virtual MaterialInstance* AccessMaterialInstance(MaterialInstanceHandle handle) = 0;


		virtual void TransitionImageLayoutImmediate(TextureHandle handle, ImageLayout oldLayout, ImageLayout newLayout, U32 baseMipLevel, U32 mipCount, bool isDepth) = 0;
		virtual void TransitionImageLayout(CommandBuffer* cmd,TextureHandle handle, ImageLayout oldLayout, ImageLayout newLayout, U32 baseMipLevel, U32 mipCount, bool isDepth) = 0;
		virtual void ResizeTexture(TextureHandle handle, U32 width, U32 height) = 0;
		virtual void ResizeFrameBuffer(FrameBufferHandle handle, U32 width, U32 height) = 0;
		virtual bool TryTransitionImageLayout(CommandBuffer* cmd,TextureHandle texture,ImageLayout newLayout, U32 baseMipLevel, U32 mipCount) = 0;
		virtual bool UploadDataToBuffer(BufferHandle buffer, const void* dataIn, U32 size,U32 offset) = 0;
		virtual bool UploadCurrentDataToBuffer(BufferHandle buffer) = 0;
		virtual void* GetMappedBufferDataPtr(BufferHandle buffer) = 0;
		virtual Format GetDepthFormat() const = 0;
		virtual void DrawDebugWindows() = 0;
		virtual void SetCurrentFrameIndex(U32 frameIndex) = 0;
		virtual U32 GetCurrentFrameIndex() const = 0;

	};

}
