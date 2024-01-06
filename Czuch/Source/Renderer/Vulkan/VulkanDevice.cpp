#include "czpch.h"
#include "VulkanDevice.h"
#include "VulkanPipelineBuilder.h"
#include "./Renderer/CommandBuffer.h"
#include"Core/Window.h"
#include"VulkanCommandBuffer.h"
#include"VulkanCore.h"
#include"Subsystems/EventsManager.h"
#include"Events/EventsTypes/ApplicationEvents.h"
#include<set>

namespace Czuch
{
	const CzuchStr Tag = "[VulkanDevice]";

	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	const std::vector<const char*> validationLayers = {
"VK_LAYER_KHRONOS_validation"
	};

#pragma region Create methods

	Pipeline* VulkanDevice::CreatePipelineState(const PipelineStateDesc* desc, const RenderPass* rpass) const
	{
		CZUCH_BE_ASSERT(desc, "CreatePipelineState NULL desc input");
		CZUCH_BE_ASSERT(rpass, "RenderPass is null");

		Pipeline* ps = new Pipeline();
		ps->m_InternalResourceState = std::make_shared<Pipeline_Vulkan>();
		ps->m_desc = *desc;

		VulkanPipelineBuilder builder(m_Device,Internal_To_Pipeline(ps),desc);
		if (!builder.BuildPipeline(Internal_to_RenderPass(rpass)->renderPass))
		{
			LOG_BE_ERROR("[{0}] Failed to Build new pipeline",Tag);
			delete ps;
			return nullptr;
		}

		return ps;
	}

	Shader* VulkanDevice::CreateShader(ShaderStage shaderStage, const char* shaderCode, size_t shaderCodeSize) const
	{
		CZUCH_BE_ASSERT(shaderCode, "CreateShader NULL shader code input");

		Shader* shader = new Shader();
		shader->m_InternalResourceState = std::make_shared<Shader_Vulkan>();
		shader->stage = shaderStage;

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCodeSize;
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode);

		Shader_Vulkan* vulkan_shader = Internal_To_Shader(shader);
		vulkan_shader->device = m_Device;

		if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &vulkan_shader->shaderModule) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to create new shader module",Tag);
			delete shader;
			return nullptr;
		}

		vulkan_shader->shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vulkan_shader->shaderStageInfo.module = vulkan_shader->shaderModule;
		vulkan_shader->shaderStageInfo.pName = "main";
		vulkan_shader->shaderStageInfo.stage = ConvertShaderStage(shaderStage);

		return shader;
	}

	RenderPass* VulkanDevice::CreateRenderPass(const RenderPassDesc* desc) const
	{
		RenderPass* rp = new RenderPass();
		rp->m_InternalResourceState = std::make_shared<RenderPass_Vulkan>();
		rp->desc = *desc;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapChainData.swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		RenderPass_Vulkan* rpass = Internal_to_RenderPass(rp);
		rpass->device = m_Device;

		if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &(rpass->renderPass)) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to create new render pass",Tag);
			delete rp;
			return nullptr;
		}

		return rp;
	}

	VkDescriptorSetLayoutBinding CreateBinding(const DescriptorSetLayoutDesc::Binding& binding)
	{
		VkDescriptorSetLayoutBinding vkBinding{};
		vkBinding.binding = binding.index;
		vkBinding.descriptorCount = binding.count;
		vkBinding.descriptorType = ConvertDescriptorType(binding.type);
		vkBinding.stageFlags = VK_SHADER_STAGE_ALL;
		vkBinding.pImmutableSamplers = nullptr;
		return vkBinding;
	}

	DescriptorSetLayout* VulkanDevice::CreateDescriptorSetLayout(const DescriptorSetLayoutDesc* desc) const
	{
		CZUCH_BE_ASSERT(desc != nullptr, "Invalid descriptor set layout desc.");

		DescriptorSetLayout* dsl = new DescriptorSetLayout();
		dsl->m_InternalResourceState = std::make_shared<DescriptorSetLayout_Vulkan>();
		dsl->desc = *desc;

		VkDescriptorSetLayoutBinding bindingsArray[s_max_descriptors_per_set];

		for (int a = 0; a < desc->bindingsCount; a++)
		{
			bindingsArray[a] = CreateBinding(desc->bindings[a]);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = desc->bindingsCount;
		layoutInfo.pBindings = &bindingsArray[0];

		DescriptorSetLayout_Vulkan* dslayout = Internal_to_DescriptorSetLayout(dsl);
		dslayout->device = m_Device;

		if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &dslayout->layout) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to create new descriptor set layout", Tag);
			delete dsl;
			return nullptr;
		}

		return dsl;
	}

	FrameBuffer* VulkanDevice::CreateFrameBuffer(const FrameBufferDesc* desc) const
	{
		CZUCH_BE_ASSERT(desc != nullptr, "Invalid frame buffer desc.");

		FrameBuffer*  fb = new FrameBuffer();
		fb->m_InternalResourceState = std::make_shared<FrameBuffer_Vulkan>();
		fb->desc = *desc;

		CZUCH_BE_ASSERT(desc->texture != nullptr, "Invalid frame buffer desc's texture.");

		auto vulkanImage = Internal_to_Image(desc->texture);

		VkImageView attachments[] = {
			vulkanImage->imageView
		};

		CZUCH_BE_ASSERT(desc->renderPass != nullptr, "Invalid frame buffer desc's render pass.");
		U32 width = desc->texture->desc.width;
		U32 height = desc->texture->desc.height;

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = Internal_to_RenderPass(desc->renderPass)->renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		auto vulkanFramebuffer = Internal_to_Framebuffer(fb);
		vulkanFramebuffer->createInfo = framebufferInfo;
		vulkanFramebuffer->device = m_Device;

		if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &vulkanFramebuffer->framebuffer) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to create new frame buffer", Tag);
			delete fb;
			return nullptr;
		}

		return fb;
	}

	CommandBuffer* VulkanDevice::CreateCommandBuffer(bool isPrimary) const
	{
		VulkanCommandBuffer* cmdBuffer = nullptr;
		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to create new command buffer", Tag);
			return nullptr;
		}

		cmdBuffer = new VulkanCommandBuffer(commandBuffer);
		cmdBuffer->Init(this);

		return cmdBuffer;;
	}

#pragma endregion

#pragma region Release

	bool VulkanDevice::ReleasePipeline(Pipeline* pipeline) const
	{
		CZUCH_BE_ASSERT(pipeline != nullptr, "Invalid pipeline passed to release.");
		delete pipeline;
		pipeline = nullptr;
		return true;
	}

	bool VulkanDevice::ReleaseShader(Shader* shader) const
	{
		CZUCH_BE_ASSERT(shader != nullptr, "Invalid shader passed to release.");
		delete shader;
		shader = nullptr;
		return true;
	}

	bool VulkanDevice::ReleaseRenderPass(RenderPass* rp) const
	{
		CZUCH_BE_ASSERT(rp != nullptr, "Invalid render pass passed to release.");
		delete rp;
		rp = nullptr;
		return true;
	}

	bool VulkanDevice::ReleaseDescriptorSetLayout(DescriptorSetLayout* dsl) const
	{
		CZUCH_BE_ASSERT(dsl != nullptr, "Invalid descriptor set layout to release");
		delete dsl;
		dsl = nullptr;
		return true;
	}

	bool VulkanDevice::ReleaseFrameBuffer(FrameBuffer* fb) const
	{
		CZUCH_BE_ASSERT(fb != nullptr, "Invalid frame buffer to release");
		delete fb;
		fb = nullptr;
		return true;
	}

	bool VulkanDevice::ReleaseCommandBuffer(CommandBuffer* cb) const
	{
		CZUCH_BE_ASSERT(cb != nullptr, "Invalid frame buffer to release");
		cb->Release();
		delete cb;
		cb = nullptr;
		return true;
	}

	VkSemaphore VulkanDevice::CreateNewSemaphore()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkSemaphore semaphore;
		if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} Failed to create new semaphore", Tag);
			return nullptr;
		}
		return semaphore;
	}

	void VulkanDevice::ReleaseSemaphore(VkSemaphore sem)
	{
		vkDestroySemaphore(m_Device, sem, nullptr);
	}

	VkFence VulkanDevice::CreateNewFence(bool signaledState)
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		if (signaledState)
		{
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		}
		VkFence fence;
		if (vkCreateFence(m_Device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} Failed to create new fence", Tag);
			return nullptr;
		}
		return fence;
	}

	void VulkanDevice::ReleaseFence(VkFence fence)
	{
		vkDestroyFence(m_Device, fence, nullptr);
	}

	void VulkanDevice::Present(uint32_t imageIndex, VkSemaphore semaphore)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &semaphore;

		VkSwapchainKHR swapChains[] = { m_SwapChainData.swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;
		VkResult result=vkQueuePresentKHR(m_PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FrameBufferResized) {
			m_FrameBufferResized = false;
			RecreateSwapChain();
		}
	}

	void VulkanDevice::BindSwapChainRenderPass(CommandBuffer* cmdBuffer,uint32_t imageIndex)
	{
		cmdBuffer->BindPass(m_SwapChainRenderPass,m_SwapChainData.swapChainFrameBuffers[imageIndex]);
	}

	void VulkanDevice::SubmitToGraphicsQueue(VkSubmitInfo info, VkFence fence)
	{
		if (vkQueueSubmit(m_GraphicsQueue, 1, &info, fence) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to submit to graphics queue", Tag);
		}
	}

	uint32_t VulkanDevice::AcquireNextSwapChainImage(VkSemaphore sem,bool& aquireFailed)
	{
		uint32_t imageIndex;
		VkResult result= vkAcquireNextImageKHR(m_Device,m_SwapChainData.swapChain, UINT64_MAX, sem, VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			aquireFailed = true;
			return 0;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			aquireFailed = true;
			return 0;
		}
		
		aquireFailed = false;
		return imageIndex;
	}

	bool VulkanDevice::ReleaseSwapChainRenderPass()
	{
		delete m_SwapChainRenderPass;
		m_SwapChainRenderPass = nullptr;

		return true;
	}

#pragma region helpers

	VkImageView VulkanDevice::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;

		if (vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} failed to create texture image view!", Tag);
			return nullptr;
		}
		return imageView;
	}


	VulkanDevice::QueueFamiliesIndexes VulkanDevice::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamiliesIndexes families;

		uint32_t familiesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &familiesCount, nullptr);
		std::vector<VkQueueFamilyProperties> properties(familiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &familiesCount, properties.data());

		//find family with graphics support
		int i = 0;
		for (const auto& family : properties)
		{
			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				families.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport)
			{
				families.presentFamily = i;
			}

			if (families.IsFullfilled())
			{
				break;
			}

			i++;
		}

		return families;
	}


	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
		auto maxVal = (std::numeric_limits<uint32_t>::max)();
		if (capabilities.currentExtent.width != maxVal) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	VulkanDevice::SwapChainSupportDetails VulkanDevice::QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails swapChainDetails{};

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainDetails.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			swapChainDetails.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, swapChainDetails.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			swapChainDetails.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, swapChainDetails.presentModes.data());
		}

		return swapChainDetails;
	}

	void VulkanDevice::OnEvent(const Czuch::Event& e)
	{
		if (e.GetEventType() == WindowSizeChangedEvent::GetStaticEventType())
		{
			m_FrameBufferResized = true;
		}
	}

	bool VulkanDevice::RecreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize((GLFWwindow*)m_AttachedWindow->GetNativeWindowPtr(), &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize((GLFWwindow*)m_AttachedWindow->GetNativeWindowPtr(), &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_Device);
		m_SwapChainData.Release(m_Device);

		CreateSwapChain();
		CreateSwapChainFrameBuffers(false);
		
		return true;
	}

	bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		auto familesIndexes = FindQueueFamilies(device, surface);
		bool extensionsSupported = CheckDeviceExtensionsSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapDetails = QuerySwapChainSupport(device, surface);
			swapChainAdequate = !swapDetails.formats.empty() && !swapDetails.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return familesIndexes.IsFullfilled() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	std::vector<const char*> VulkanDevice::GetRequiredExtensions()
	{
		uint32_t extensionsCount = 0;
		const char** glfwExtensions = nullptr;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionsCount);

		if (m_RendererValidationMode == ValidationMode::Enabled)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	bool VulkanDevice::CheckDeviceExtensionsSupport(VkPhysicalDevice device)
	{
		uint32_t deviceExtensionsCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionsCount, nullptr);
		std::vector<VkExtensionProperties> properties(deviceExtensionsCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionsCount, properties.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& property : properties)
		{
			requiredExtensions.erase(property.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool VulkanDevice::HasValidationLayerSupport()
	{
		uint32_t layersCount;
		vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
		std::vector<VkLayerProperties> layers(layersCount);
		vkEnumerateInstanceLayerProperties(&layersCount, layers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : layers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}



#pragma endregion 

#pragma endregion


	VulkanDevice::VulkanDevice(Window* window, ValidationMode validationMode):m_AttachedWindow(window),m_FrameBufferResized(false)
	{
		m_RendererValidationMode = validationMode;
	}


	VulkanDevice::~VulkanDevice()
	{	
		EventsManager::Get().RemoveListener(WindowSizeChangedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
		m_SwapChainData.Release(m_Device);
		ReleaseSwapChainRenderPass();
		vkDestroyDevice(m_Device, nullptr);
		if (m_Surface != nullptr)
		{
			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		}
		vkDestroyInstance(m_Instance, nullptr);
	}


	bool VulkanDevice::InitDevice()
	{
		if (CreateVulkanInstance() == false)
		{
			return false;
		}

		if (CreateSurface() == false)
		{
			return false;
		}

		if (SelectPhysicalDevice() == false)
		{
			return false;
		}

		if (CreateLogicalDevice() == false)
		{
			return false;
		}

		if (CreateSwapChain() == false)
		{
			return false;
		}

		if (CreateSwapChainFrameBuffers() == false)
		{
			return false;
		}

		if (CreateCommandsPool() == false)
		{
			return false;
		}

		EventsManager::Get().AddListener(WindowSizeChangedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);

		return true;
	}

	bool VulkanDevice::CreateVulkanInstance()
	{
		if (m_RendererValidationMode == ValidationMode::Enabled && HasValidationLayerSupport() == false)
		{
			LOG_BE_ERROR("{0} Validation mode enabled but not available.", Tag);
			return false;
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "CzuchEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.applicationVersion = VK_API_VERSION_1_3;
		appInfo.pEngineName = "Czuch";

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = GetRequiredExtensions();

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (this->m_RendererValidationMode == ValidationMode::Enabled)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		uint32_t vkExtensionsCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionsCount, nullptr);
		std::vector<VkExtensionProperties> extensionsVector(vkExtensionsCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionsCount, extensionsVector.data());

		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

		if (result != VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} Failed to create vulkan instance.", Tag);
			return false;
		}
		return true;
	}

	bool VulkanDevice::CreateSurface()
	{
		if (m_AttachedWindow == nullptr)
		{
			LOG_BE_WARN("{0} Window surface won't be created because there is no window attached.", Tag);
			return false;
		}

		if (glfwCreateWindowSurface(m_Instance, (GLFWwindow*)m_AttachedWindow->GetNativeWindowPtr(), nullptr, &m_Surface)!=VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} Failed to create vulkan window surface.", Tag);
			return false;
		}
		return true;
	}

	bool VulkanDevice::CreateLogicalDevice()
	{
		QueueFamiliesIndexes families = FindQueueFamilies(m_PhysicalDevice, m_Surface);

		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		std::set<uint32_t> indices = { families.graphicsFamily.value(),families.presentFamily.value() };

		float priority = 1.0f;
		for (const auto& elem : indices)
		{
			VkDeviceQueueCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			info.queueCount = 1;
			info.queueFamilyIndex = elem;
			info.pQueuePriorities = &priority;
			queueInfos.push_back(info);
		}


		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueInfos.data();
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (m_RendererValidationMode == ValidationMode::Enabled)
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} Failed to Create Vulkan Device.", Tag);
			return false;
		}

		vkGetDeviceQueue(m_Device, families.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, families.presentFamily.value(), 0, &m_PresentQueue);
		return true;
	}

	bool VulkanDevice::CreateSwapChain()
	{
		m_SwapChainData = {};
		SwapChainSupportDetails swapchainDetails = QuerySwapChainSupport(m_PhysicalDevice, m_Surface);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainDetails.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainDetails.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapchainDetails.capabilities,(GLFWwindow*)m_AttachedWindow->GetNativeWindowPtr());

		m_SwapChainData.swapChainExtent = extent;
		m_SwapChainData.swapChainImageFormat = surfaceFormat.format;

		uint32_t imagesCount = swapchainDetails.capabilities.minImageCount + 1;

		if (swapchainDetails.capabilities.maxImageCount > 0 && swapchainDetails.capabilities.maxImageCount < imagesCount)
		{
			imagesCount = swapchainDetails.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = imagesCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //when postprocessing added it should change to VK_IMAGE_USAGE_TRANSFER_DST_BIT and then we just copy render target texture to swap chaing after all postprocessing is done

		QueueFamiliesIndexes indices = FindQueueFamilies(m_PhysicalDevice, m_Surface);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapchainDetails.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChainData.swapChain) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to Create Swap chain.", Tag);
			return false;
		}

		vkGetSwapchainImagesKHR(m_Device, m_SwapChainData.swapChain, &imagesCount, nullptr);
		m_SwapChainData.swapChainImages.resize(imagesCount);
		vkGetSwapchainImagesKHR(m_Device, m_SwapChainData.swapChain, &imagesCount, m_SwapChainData.swapChainImages.data());

		m_SwapChainData.swapChainImageViews.resize(m_SwapChainData.swapChainImages.size());

		for (size_t i = 0; i < m_SwapChainData.swapChainImageViews.size(); i++) {
			m_SwapChainData.swapChainImageViews[i] = CreateImageView(m_SwapChainData.swapChainImages[i], m_SwapChainData.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		return true;
	}

	bool VulkanDevice::SelectPhysicalDevice()
	{
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		uint32_t devicesCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &devicesCount, nullptr);

		if (devicesCount == 0)
		{
			LOG_BE_ERROR("{0} There is no single graphics card to use.", Tag);
			return false;
		}

		std::vector<VkPhysicalDevice> devices(devicesCount);
		vkEnumeratePhysicalDevices(m_Instance, &devicesCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device, m_Surface))
			{
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == nullptr)
		{
			LOG_BE_ERROR("{0} Cannot find suitable device.", Tag);
			return false;
		}

		m_PhysicalDevice = physicalDevice;
		return true;
	}

	bool VulkanDevice::CreateSwapChainFrameBuffers(bool createRenderPass)
	{
		m_SwapChainData.swapChainFrameBuffers.resize(m_SwapChainData.swapChainImageViews.size());

		//create swap chain render pass
		if (createRenderPass==true)
		{
			RenderPassDesc desc{};
			m_SwapChainRenderPass = CreateRenderPass(&desc);
		}

		RenderPass_Vulkan* rpass = Internal_to_RenderPass(m_SwapChainRenderPass);

		for (size_t i = 0; i < m_SwapChainData.swapChainImageViews.size(); i++) {

			VkImageView attachments[] = {
				m_SwapChainData.swapChainImageViews[i]
			};

			FrameBuffer* framebuffer = new FrameBuffer();
			framebuffer->m_InternalResourceState = std::make_shared<FrameBuffer_Vulkan>();
			m_SwapChainData.swapChainFrameBuffers[i] = framebuffer;

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = rpass->renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_SwapChainData.swapChainExtent.width;
			framebufferInfo.height = m_SwapChainData.swapChainExtent.height;
			framebufferInfo.layers = 1;

			FrameBuffer_Vulkan* fbuffer = Internal_to_Framebuffer(framebuffer);
			fbuffer->device = m_Device;

			if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr,&fbuffer->framebuffer) != VK_SUCCESS) {
				LOG_BE_ERROR("{0} Failed to Create Swap chain frame buffers.", Tag);
				return false;
			}
		}

		return true;
	}


	bool VulkanDevice::CreateCommandsPool()
	{
		QueueFamiliesIndexes queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, m_Surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to create command pool.", Tag);
			return false;
		}
		return true;
	}

}
