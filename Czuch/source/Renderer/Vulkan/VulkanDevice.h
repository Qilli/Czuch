#pragma once
#include"Renderer/GraphicsDevice.h"
#include"VulkanBase.h"
#include"Events/IEventsListener.h"
#include"Renderer/GraphicsResourceAccessContainer.h"
#include "DescriptorAllocator.h"
#include<optional>

namespace Czuch
{
	class Window;
	class DescriptorLayoutCache;
	class DescriptorAllocator;
	struct BufferInternalSettings;
	struct Buffer_Vulkan;
	class VulkanCommandBuffer;


	struct ImageWithAllocation
	{
		VkImage image;
		VmaAllocation allocation;
	};

	static const U32 k_max_resources = 5000;

	class VulkanDevice final : public GraphicsDevice, public Czuch::IEventsListener
	{
	public:
		VulkanDevice(Window* window);
		~VulkanDevice() override;

		bool InitDevice(EngineSettings* settings) override;
		float GetSwapchainWidth() const override { return m_SwapChainData.swapChainExtent.width; }
		float GetSwapchainHeight() const override { return m_SwapChainData.swapChainExtent.height; }
		U32 GetSwapchainImageCount() const { return m_SwapChainData.swapChainImages.size(); }
		VkFormat* GetSwapchainFormat() { return &m_SwapChainData.swapChainImageFormat; }
		VkFormat GetNativeDepthFormat() { return m_DepthImage.depthFormat; }

		Format GetDepthFormat() const override;

		void DrawUI(CommandBuffer* commandBuffer) override;
		void PreDrawFrame();
		void* CreatePointerForUITexture(TextureHandle tex) override;

		PipelineHandle CreatePipelineState(const MaterialPassDesc* desc, RenderPass* rpass, bool dynamicRendering = false) override;
		ShaderHandle CreateShader(ShaderStage shaderStage, const char* shaderCode, size_t shaderCodeSize)override;
		RenderPassHandle CreateRenderPass(const RenderPassDesc* desc) override;
		DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutDesc* desc)  override;
		FrameBufferHandle CreateFrameBuffer(const FrameBufferDesc* desc, bool resize = false, FrameBufferHandle handle = INVALID_HANDLE(FrameBufferHandle)) override;
		CommandBufferHandle CreateCommandBuffer(bool isPrimary, void* pool = nullptr) override;
		BufferHandle CreateBuffer(const BufferDesc* desc) override;
		TextureHandle CreateTexture(const TextureDesc* desc, bool resize = false, TextureHandle handle = INVALID_HANDLE(TextureHandle)) override;
		MeshHandle CreateMesh(MeshData& meshData) override;
		MaterialHandle CreateMaterial(MaterialDefinitionDesc* materialData) override;
		MaterialInstanceHandle CreateMaterialInstance(MaterialInstanceDesc& materialInstanceDesc) override;
		BufferHandle CreateUBOBuffer(MaterialCustomBufferData* ubo) override;
		BufferHandle CreateSSBOBuffer(U32 elementsCount, U32 elemSize, bool permaMapped = true) override;


		DescriptorAllocator* CreateDescriptorAllocator();

		void AwaitDevice() const;

		void ReleaseDescriptorAllocator(DescriptorAllocator* allocator);
		bool Release(PipelineHandle& pipeline)  override;
		bool Release(ShaderHandle& shader) override;
		bool Release(RenderPassHandle& rp) override;
		bool Release(DescriptorSetLayoutHandle& dsl)  override;
		bool Release(FrameBufferHandle& fb)  override;
		bool Release(CommandBufferHandle& cb) override;
		bool Release(BufferHandle& buffer)  override;
		bool Release(TextureHandle& color_texture) override;
		bool Release(MeshHandle& mesh) override;
		bool Release(MaterialHandle& material) override;
		bool Release(MaterialInstanceHandle& materialInstance) override;

		void SetCurrentFrameIndex(U32 frameIndex) override  {
			m_CurrentFrameInFlight = frameIndex;
		}

		U32 GetCurrentFrameIndex() const override { return m_CurrentFrameInFlight; }


		Pipeline* AccessPipeline(PipelineHandle handle) override;
		RenderPass* AccessRenderPass(RenderPassHandle handle) override;
		RenderPass* GetRenderPassOfType(RenderPassType type) override;
		Shader* AccessShader(ShaderHandle handle)  override;
		DescriptorSetLayout* AccessDescriptorSetLayout(DescriptorSetLayoutHandle handle) override;
		FrameBuffer* AccessFrameBuffer(FrameBufferHandle handle) override;
		CommandBuffer* AccessCommandBuffer(CommandBufferHandle handle)  override;
		Buffer* AccessBuffer(BufferHandle handle)  override;
		Texture* AccessTexture(TextureHandle handle) override;
		Mesh* AccessMesh(MeshHandle handle) override;
		Material* AccessMaterial(MaterialHandle handle) override;
		MaterialInstance* AccessMaterialInstance(MaterialInstanceHandle handle) override;

		void ResizeTexture(TextureHandle texture, U32 width, U32 height) override;
		void ResizeFrameBuffer(FrameBufferHandle handle, U32 width, U32 height) override;
	public:
		void TransitionSwapChainImageLayoutPreDraw(VulkanCommandBuffer* cmd, uint32_t imageIndex);
		void TransitionSwapChainImageLayoutPostDraw(VulkanCommandBuffer* cmd, uint32_t imageIndex);
	public:
		void* InitImGUI();
		void ShutdownImGUI();
		void TransitionImageLayoutImmediate(TextureHandle handle, ImageLayout oldLayout, ImageLayout newLayout, U32 baseMipLevel, U32 mipCount, bool isDepth) override;
		void TransitionImageLayout(CommandBuffer* cmd, TextureHandle handle, ImageLayout oldLayout, ImageLayout newLayout, U32 baseMipLevel, U32 mipCount, bool isDepth) override;
		bool TryTransitionImageLayout(CommandBuffer* cmd, TextureHandle texture, ImageLayout newLayout, U32 baseMipLevel, U32 mipCount) override;
	public:
		bool UploadDataToBuffer(BufferHandle buffer, const void* dataIn, U32 size, U32 offset) override;
		bool UploadCurrentDataToBuffer(BufferHandle buffer) override;
		void* GetMappedBufferDataPtr(BufferHandle buffer) override;
	public:
		VkDevice GetNativeDevice() const { return m_Device; }
		VmaAllocator GetAllocator() const { return m_VmaAllocator; }
		int GetCurrentImageIndex() const { return m_CurrentImageIndex; }
		VkSemaphore CreateNewSemaphore();
		void ReleaseSemaphore(VkSemaphore sem);
		VkFence CreateNewFence(bool signaledState = false);
		void ReleaseFence(VkFence fence);
		VkCommandPool CreateCommandPool(bool isTransient, bool isResettable);
		VkCommandBufferSubmitInfo CreateCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer);
		VkSubmitInfo2 CreateSubmitInfo(VkCommandBufferSubmitInfo* cmdBufferSubmitInfo, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);
		void ReleaseCommandPool(VkCommandPool pool);
		uint32_t AcquireNextSwapChainImage(VkSemaphore sem, bool& aquireFailed);
		void SubmitToGraphicsQueue(VkSubmitInfo info, VkFence fence);
		void ImmediateSubmitToGraphicsQueueWithCommandBuffer(VkCommandBuffer cmdBuffer, VkFence fence);
		void Present(uint32_t imageIndex, VkSemaphore semaphore);
		void BindSwapChainRenderPass(CommandBuffer* cmdBuffer, uint32_t imageIndex);
	public:
		void StartDynamicRenderPass(VulkanCommandBuffer* cmdBuffer, uint32_t imageIndex);
		bool HasDynamicRenderingEnabled() const;
		RenderPassHandle GetSwapChainRenderPass() const { return m_SwapChainRenderPass; }
		DescriptorSetLayoutHandle GetBindlessDescriptorSetLayout() const { return m_BindlessDescriptorSetLayoutHandle; }
		DescriptorSet* GetBindlessTexturesDescriptorSet() const { return m_TexturesBindlessDescriptorSet; }
	public:
		void DrawDebugWindows() override ; 
		void AddDebugWindow(DrawDebugBaseGuiWindow* window) override;
	private:

		struct DebugWindowsContainer
		{
			Array<DrawDebugBaseGuiWindow*> windows;
			void DrawAll();
		};

		DebugWindowsContainer m_DebugWindows;

	private:

		struct DepthImage
		{
			VkImage depthImage;
			VmaAllocation allocation;
			VkImageView depthImageView;
			VkFormat depthFormat;

			void Release(VkDevice device, VmaAllocator allocator);
		};

		struct SwapChainData
		{
			std::vector<VkImage> swapChainImages;
			std::vector<VkImageView> swapChainImageViews;
			std::vector<FrameBufferHandle> swapChainFrameBuffers;
			VkExtent2D swapChainExtent;
			VkFormat swapChainImageFormat;
			VkSwapchainKHR swapChain;

			void Release(VkDevice device)
			{
				for (uint32_t i = 0; i < swapChainImageViews.size(); ++i)
				{
					vkDestroyImageView(device, swapChainImageViews[i], nullptr);
				}
				vkDestroySwapchainKHR(device, swapChain, nullptr);

				swapChainImages.clear();
				swapChainImageViews.clear();
				swapChainFrameBuffers.clear();
			}
		};

		struct QueueFamiliesIndexes
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool IsFullfilled()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		struct ResourcesContainer
		{
			GraphicsResourceAccessContainer<Texture, TextureHandle> textures = GraphicsResourceAccessContainer<Texture, TextureHandle>(k_max_resources);
			GraphicsResourceAccessContainer<Buffer, BufferHandle> buffers = GraphicsResourceAccessContainer<Buffer, BufferHandle>(k_max_resources);
			GraphicsResourceAccessContainer<CommandBuffer, CommandBufferHandle> commandBuffers = GraphicsResourceAccessContainer<CommandBuffer, CommandBufferHandle>(k_max_resources);
			GraphicsResourceAccessContainer<Pipeline, PipelineHandle> pipelines = GraphicsResourceAccessContainer<Pipeline, PipelineHandle>(k_max_resources);
			GraphicsResourceAccessContainer<RenderPass, RenderPassHandle> renderPasses = GraphicsResourceAccessContainer<RenderPass, RenderPassHandle>(k_max_resources);
			GraphicsResourceAccessContainer<Shader, ShaderHandle> shaders = GraphicsResourceAccessContainer<Shader, ShaderHandle>(k_max_resources);
			GraphicsResourceAccessContainer<FrameBuffer, FrameBufferHandle> frameBuffers = GraphicsResourceAccessContainer<FrameBuffer, FrameBufferHandle>(k_max_resources);
			GraphicsResourceAccessContainer<DescriptorSetLayout, DescriptorSetLayoutHandle> descriptorSetLayouts = GraphicsResourceAccessContainer<DescriptorSetLayout, DescriptorSetLayoutHandle>(k_max_resources);
			GraphicsResourceAccessContainer<Mesh, MeshHandle> meshes = GraphicsResourceAccessContainer<Mesh, MeshHandle>(k_max_resources);
			GraphicsResourceAccessContainer<Material, MaterialHandle> materials = GraphicsResourceAccessContainer<Material, MaterialHandle>(k_max_resources);
			GraphicsResourceAccessContainer<MaterialInstance, MaterialInstanceHandle> materialInstances = GraphicsResourceAccessContainer<MaterialInstance, MaterialInstanceHandle>(k_max_resources);
			void ReleaseAll();
		};

	private:
		ResourcesContainer m_ResContainer;
		MultiplerBufferContainer m_MultipleBuffers;
	private:
		Window* m_AttachedWindow;
		int m_CurrentImageIndex;

		VkInstance m_Instance;
		VkSurfaceKHR m_Surface;
		VkDevice m_Device;
		VkPhysicalDevice m_PhysicalDevice;
		VkQueue m_PresentQueue;
		VkQueue m_GraphicsQueue;

		SwapChainData m_SwapChainData;
		VkCommandPool m_CommandPool;
		VkCommandPool m_CopyCommandPool;
		RenderPassHandle m_SwapChainRenderPass;

		VkPhysicalDeviceMemoryProperties m_MemProperties;

		bool m_FrameBufferResized;

		VmaAllocator m_VmaAllocator;
		DescriptorLayoutCache* m_DescriptorLayoutCache;
		DescriptorAllocator* m_PersistentDescriptorAllocator;
		VkPhysicalDeviceProperties m_DeviceProperties;

		DepthImage m_DepthImage;
		U32 m_CurrentFrameInFlight;
	private:
		DescriptorSetLayoutHandle m_BindlessDescriptorSetLayoutHandle;
		DescriptorSet* m_TexturesBindlessDescriptorSet;
		DescriptorWriter m_BindlessDescriptorSetWriter;
		U32 m_GlobalTexturesCount;
		VkSampler m_GlobalSampler;
	public:
		TextureHandle BindGlobalTexture(TextureHandle texture) override;
	private:
		VkDescriptorPool m_ImguiPool;
	private:
		RenderPassHandle GetRenderPassWithDescIfExist(const RenderPassDesc* desc);
	private:
		bool CreateVulkanInstance();
		bool CreateSurface();
		bool CreateLogicalDevice();
		bool SelectPhysicalDevice();
		bool CreateAllocatorObject();
		bool CreateCommandsPool();

	private:
		bool CreateDepthImage();
		bool CreateSwapChain();
		bool CreateSwapChainFrameBuffers(bool createRenderPass = true);
		bool RecreateSwapChain();
	private:
		U32 FindMemoryType(U32 typeFilter, VkMemoryPropertyFlags properties) const;
		VkDeviceMemory AllocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags memoryProperty) const;
		bool CreateBuffer_Internal(BufferInternalSettings& settings) const;
		bool CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize dstOffset) const;
		bool CopyDataToBuffer(Buffer_Vulkan* buffer, const void* dataIn, U32 size, U32 offset) const;
		void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, U32 w, U32 h) const;
		void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkFormat format, VkImageLayout currentLayout, VkImageLayout targetLayout, U32 baseMipLevel, U32 mipCount, bool isDepth) const;
		void DoImageMemoryBarrier(VkCommandBuffer cmdbuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange) const;
		VkCommandBuffer BeginSingleTimeCommands() const;
		void EndSingleTimeCommands(VkCommandBuffer cmd) const;
	private:
		void GenerateMipmaps(VkImage image, VkFormat imageFormat, U32 texWidth, U32 texHeight, U32 mipLevels);
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, int mipsCount) const;
		ImageWithAllocation CreateImage(TextureDesc::Type type, U32 width, U32 height, U32 mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageLayout initLayout, VkSharingMode sharingMode) const;
		VkSampler CreateImageSampler(const SamplerDesc& desc) const;
	private:
		virtual void OnEvent(Czuch::Event& e) override;

	private:
		bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
		bool HasValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		bool CheckDeviceExtensionsSupport(VkPhysicalDevice device);
		QueueFamiliesIndexes FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	};

}

