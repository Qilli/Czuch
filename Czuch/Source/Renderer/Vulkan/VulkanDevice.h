#pragma once
#include"Renderer/GraphicsDevice.h"
#include"VulkanBase.h"
#include"Events/IEventsListener.h"
#include<optional>

namespace Czuch
{
	class Window;

	class VulkanDevice final : public GraphicsDevice, public Czuch::IEventsListener
	{
	public:
		VulkanDevice(Window* window,ValidationMode validationMode = ValidationMode::Disabled);
		~VulkanDevice() override;

		bool InitDevice();
		float GetSwapchainWidth() const override { return m_SwapChainData.swapChainExtent.width; }
		float GetSwapchainHeight() const override { return m_SwapChainData.swapChainExtent.height; }

		Pipeline* CreatePipelineState(const PipelineStateDesc* desc, const RenderPass* rpass) const override;
		Shader* CreateShader(ShaderStage shaderStage, const char* shaderCode, size_t shaderCodeSize) const override;
		RenderPass* CreateRenderPass(const RenderPassDesc* desc) const override;
		DescriptorSetLayout* CreateDescriptorSetLayout(const DescriptorSetLayoutDesc* desc) const override;
		FrameBuffer* CreateFrameBuffer(const FrameBufferDesc* desc) const override;
		CommandBuffer* CreateCommandBuffer(bool isPrimary) const override;
		Buffer* CreateBuffer(const BufferDesc* desc) const override;


		bool ReleasePipeline(Pipeline* pipeline) const override;
		bool ReleaseShader(Shader* shader) const override;
		bool ReleaseRenderPass(RenderPass* rp) const override;
		bool ReleaseDescriptorSetLayout(DescriptorSetLayout* dsl) const override;
		bool ReleaseFrameBuffer(FrameBuffer* fb) const override;
		bool ReleaseCommandBuffer(CommandBuffer* cb)const override;
		bool ReleaseBuffer(Buffer* buffer) const override;
	public:
		VkDevice GetNativeDevice() const { return m_Device; }
		VkSemaphore CreateNewSemaphore();
		void ReleaseSemaphore(VkSemaphore sem);
		VkFence CreateNewFence(bool signaledState=false);
		void ReleaseFence(VkFence fence);
		uint32_t AcquireNextSwapChainImage(VkSemaphore sem,bool& aquireFailed);
		void SubmitToGraphicsQueue(VkSubmitInfo info, VkFence fence);
		void Present(uint32_t imageIndex, VkSemaphore semaphore);
		void BindSwapChainRenderPass(CommandBuffer* cmdBuffer, uint32_t imageIndex);
	private:

		struct SwapChainData
		{
			std::vector<VkImage> swapChainImages;
			std::vector<VkImageView> swapChainImageViews;
			std::vector<FrameBuffer*> swapChainFrameBuffers;
			VkExtent2D swapChainExtent;
			VkFormat swapChainImageFormat;
			VkSwapchainKHR swapChain;

			void Release(VkDevice device)
			{
				for (uint32_t i = 0; i < swapChainImageViews.size(); ++i)
				{
					delete swapChainFrameBuffers[i];
				}

				for (uint32_t i = 0; i < swapChainImageViews.size(); ++i)
				{
					vkDestroyImageView(device,swapChainImageViews[i],nullptr);
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

	private:
		Window* m_AttachedWindow;

		VkInstance m_Instance;
		VkSurfaceKHR m_Surface;
		VkDevice m_Device;
		VkPhysicalDevice m_PhysicalDevice;
		VkQueue m_PresentQueue;
		VkQueue m_GraphicsQueue;

		SwapChainData m_SwapChainData;
		VkCommandPool m_CommandPool;
		RenderPass* m_SwapChainRenderPass;

		bool m_FrameBufferResized;

	private:
		bool CreateVulkanInstance();
		bool CreateSurface();
		bool CreateLogicalDevice();
		bool SelectPhysicalDevice();
		bool CreateCommandsPool();

	private:
		bool CreateSwapChain();
		bool CreateSwapChainFrameBuffers(bool createRenderPass=true);
		bool ReleaseSwapChainRenderPass();
		bool RecreateSwapChain();
	private:
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	private:
		virtual void OnEvent(const Czuch::Event& e) override;
		
	private:
		bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
		bool HasValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		bool CheckDeviceExtensionsSupport(VkPhysicalDevice device);
		QueueFamiliesIndexes FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	};

}

