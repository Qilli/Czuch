#include "czpch.h"
#include "VulkanDevice.h"
#include "VulkanPipelineBuilder.h"
#include "./Renderer/CommandBuffer.h"
#include"Core/Window.h"
#include"VulkanCommandBuffer.h"
#include"VulkanCore.h"
#include"Subsystems/EventsManager.h"
#include"Events/EventsTypes/ApplicationEvents.h"
#include"Core/Common.h"
#include"DescriptorLayoutCache.h"
#include"DescriptorAllocator.h"
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

	PipelineHandle VulkanDevice::CreatePipelineState(const PipelineStateDesc* desc, const RenderPassHandle rpass)
	{
		CZUCH_BE_ASSERT(desc, "CreatePipelineState NULL desc input");
		
		RenderPass* rp = nullptr;
		if (!HANDLE_IS_VALID(rpass))
		{
			rp = AccessRenderPass(m_SwapChainRenderPass);
		}
		else
		{
			m_ResContainer.renderPasses.Get(rpass.handle, &rp);
		}

		Pipeline* ps = new Pipeline();
		ps->m_InternalResourceState = std::make_shared<Pipeline_Vulkan>();
		ps->m_desc = *desc;

		VulkanPipelineBuilder builder(this,Internal_To_Pipeline(ps),desc);
		if (!builder.BuildPipeline(Internal_to_RenderPass(rp)->renderPass))
		{
			LOG_BE_ERROR("[{0}] Failed to Build new pipeline",Tag);
			delete ps;
			return INVALID_HANDLE(PipelineHandle);
		}

		PipelineHandle h;
		h.handle=m_ResContainer.pipelines.Add(ps);

		return h;
	}

	ShaderHandle VulkanDevice::CreateShader(ShaderStage shaderStage, const char* shaderCode, size_t shaderCodeSize) 
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
			return INVALID_HANDLE(ShaderHandle);
		}

		vulkan_shader->shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vulkan_shader->shaderStageInfo.module = vulkan_shader->shaderModule;
		vulkan_shader->shaderStageInfo.pName = "main";
		vulkan_shader->shaderStageInfo.stage = ConvertShaderStageBits(shaderStage);

		ShaderHandle h;
		h.handle = m_ResContainer.shaders.Add(shader);

		return h;
	}

	RenderPassHandle VulkanDevice::CreateRenderPass(const RenderPassDesc* desc)
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
			return INVALID_HANDLE(RenderPassHandle);
		}

		RenderPassHandle h;
		h.handle = m_ResContainer.renderPasses.Add(rp);
		return h;
	}

	VkDescriptorSetLayoutBinding CreateBinding(const DescriptorSetLayoutDesc::Binding& binding,U32 stages)
	{
		VkDescriptorSetLayoutBinding vkBinding{};
		vkBinding.binding = binding.index;
		vkBinding.descriptorCount = binding.count;
		vkBinding.descriptorType = ConvertDescriptorType(binding.type);
		vkBinding.stageFlags = ConvertShaderStage(stages);
		vkBinding.pImmutableSamplers = nullptr;
		return vkBinding;
	}

	DescriptorSetLayoutHandle VulkanDevice::CreateDescriptorSetLayout(const DescriptorSetLayoutDesc* desc) 
	{
		CZUCH_BE_ASSERT(desc != nullptr, "Invalid descriptor set layout desc.");

		DescriptorSetLayout* dsl = new DescriptorSetLayout();
		dsl->m_InternalResourceState = std::make_shared<DescriptorSetLayout_Vulkan>();
		dsl->desc = *desc;

		VkDescriptorSetLayoutBinding bindingsArray[s_max_descriptors_per_set];

		for (U32 a = 0; a < desc->bindingsCount; a++)
		{
			bindingsArray[a] = CreateBinding(desc->bindings[a],desc->shaderStage);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = desc->bindingsCount;
		layoutInfo.pBindings = bindingsArray;

		DescriptorSetLayout_Vulkan* dslayout = Internal_to_DescriptorSetLayout(dsl);
		dslayout->device = m_Device;

		dslayout->layout=m_DescriptorLayoutCache->CreateDescriptorLayout(&layoutInfo);

		if (dslayout->layout==nullptr) {
			LOG_BE_ERROR("{0} Failed to create new descriptor set layout", Tag);
			delete dsl;
			return INVALID_HANDLE(DescriptorSetLayoutHandle);
		}


		DescriptorSetLayoutHandle h;
		h.handle = m_ResContainer.descriptorSetLayouts.Add(dsl);
		return h;
	}

	FrameBufferHandle VulkanDevice::CreateFrameBuffer(const FrameBufferDesc* desc) 
	{
		CZUCH_BE_ASSERT(desc != nullptr, "Invalid frame buffer desc.");

		FrameBuffer*  fb = new FrameBuffer();
		fb->m_InternalResourceState = std::make_shared<FrameBuffer_Vulkan>();
		fb->desc = *desc;

		CZUCH_BE_ASSERT(desc->texture != nullptr, "Invalid frame buffer desc's texture.");

		auto vulkanTexture = Internal_to_Texture(desc->texture);

		VkImageView attachments[] = {
			vulkanTexture->imageView
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
			return INVALID_HANDLE(FrameBufferHandle);
		}

		FrameBufferHandle h;
		h.handle = m_ResContainer.frameBuffers.Add(fb);
		return h;
	}

	CommandBufferHandle VulkanDevice::CreateCommandBuffer(bool isPrimary)
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
			return INVALID_HANDLE(CommandBufferHandle);
		}

		cmdBuffer = new VulkanCommandBuffer(commandBuffer);
		cmdBuffer->Init(this);

		CommandBufferHandle h;
		h.handle = m_ResContainer.commandBuffers.Add(cmdBuffer);
		return h;
	}

	TextureHandle VulkanDevice::CreateTexture(const TextureDesc* desc)
	{
		CZUCH_BE_ASSERT(desc != nullptr, "Invalid texture desc.");

		Texture* texture = new Texture();
		texture->desc = *desc;
		texture->m_InternalResourceState = std::make_shared<Texture_Vulkan>();

		auto vulkanTexture = Internal_to_Texture(texture);
		vulkanTexture->allocator = m_VmaAllocator;
		vulkanTexture->device = m_Device;

		BufferInternalSettings settingsStageBuffer{};
		settingsStageBuffer.inSize = desc->GetSize();
		settingsStageBuffer.inFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		settingsStageBuffer.inStagingBuffer = true;
		settingsStageBuffer.inCreateMapped = false;
		settingsStageBuffer.inUsage = Usage::MEMORY_USAGE_CPU_ONLY;

		if (!CreateBuffer_Internal(settingsStageBuffer))
		{
			LOG_BE_ERROR("{0} Failed to create staging buffer for texture copy.", Tag);
			return INVALID_HANDLE(TextureHandle);
		}

		void* data;
		vmaMapMemory(m_VmaAllocator, settingsStageBuffer.outMemAlloc, &data);
		memcpy(data, desc->texData, settingsStageBuffer.inSize);
		vmaUnmapMemory(m_VmaAllocator, settingsStageBuffer.outMemAlloc);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = ConvertImageType(desc->type);
		imageInfo.extent.width = static_cast<uint32_t>(desc->width);
		imageInfo.extent.height = static_cast<uint32_t>(desc->height);
		imageInfo.extent.depth = desc->depth;
		imageInfo.mipLevels = desc->mip_levels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = ConvertFormat(desc->format);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VmaAllocationCreateInfo memory_info{};
		memory_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;//ConvertMemoryUsage(desc->usage);

		if (vmaCreateImage(m_VmaAllocator, &imageInfo, &memory_info, &vulkanTexture->image, &vulkanTexture->allocation, nullptr)!=VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} Failed to create new vulkan image", Tag);
			return INVALID_HANDLE(TextureHandle);
		}

		//transition
		TransitionImageLayout(vulkanTexture->image, imageInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		//copy staging buffe to target image
		CopyBufferToImage(settingsStageBuffer.outBuffer, vulkanTexture->image, desc->width, desc->height);

		//transition for read in ps
		TransitionImageLayout(vulkanTexture->image, imageInfo.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vmaDestroyBuffer(m_VmaAllocator,settingsStageBuffer.outBuffer, settingsStageBuffer.outMemAlloc);

		vulkanTexture->imageView = CreateImageView(vulkanTexture->image, ConvertFormat(desc->format), VK_IMAGE_ASPECT_COLOR_BIT);
		vulkanTexture->sampler = CreateImageSampler(desc->samplerDesc);

		TextureHandle h;
		h.handle = m_ResContainer.textures.Add(texture);
		return h;
	}

	BufferHandle VulkanDevice::CreateBuffer(const BufferDesc* desc)
	{
		CZUCH_BE_ASSERT(desc != nullptr, "Invalid buffer desc.");

		Buffer* buffer = new Buffer();
		buffer->desc = *desc;
		buffer->m_InternalResourceState = std::make_shared<Buffer_Vulkan>();

		VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		if (HasFlag(desc->bind_flags, BindFlag::VERTEX_BUFFER))
		{
			usage|= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		}
		if (HasFlag(buffer->desc.bind_flags, BindFlag::INDEX_BUFFER))
		{
			usage|= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		if (HasFlag(buffer->desc.bind_flags, BindFlag::CONSTANT_BUFFER))
		{
			usage|= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		if (HasFlag(buffer->desc.bind_flags, BindFlag::SHADER_RESOURCE))
		{
			usage|= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // read only ByteAddressBuffer is also storage buffer
			usage|= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		}
		if (HasFlag(buffer->desc.bind_flags, BindFlag::UNORDERED_ACCESS))
		{
			usage|= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			usage|= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		}

		if (HasFlag(buffer->desc.bind_flags, BindFlag::UNIFORM_BUFFER))
		{
			usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}

		auto bufferVulkan = Internal_to_Buffer(buffer);
		bufferVulkan->device = m_Device;
		bufferVulkan->flags = usage;
		bufferVulkan->size = desc->size > 0 ? desc->size : 1;
		bufferVulkan->allocator = m_VmaAllocator;
		bufferVulkan->mapped = desc->createMapped;

		BufferInternalSettings settings{};
		settings.inSize = bufferVulkan->size;
		settings.inUsage = desc->usage;
		settings.inStagingBuffer = false;
		settings.inCreateMapped = desc->createMapped;
		settings.inFlags = usage;

		if (!CreateBuffer_Internal(settings)) {
			LOG_BE_ERROR("{0} Failed to create new vulkan buffer", Tag);
			return INVALID_HANDLE(BufferHandle);
		}

		bufferVulkan->buffer = settings.outBuffer;
		bufferVulkan->allocation = settings.outMemAlloc;


		if ((HasFlag(desc->bind_flags, BindFlag::VERTEX_BUFFER) || HasFlag(desc->bind_flags, BindFlag::INDEX_BUFFER)) && desc->initData!=nullptr)
		{
			BufferInternalSettings settingsStageBuffer{};
			settingsStageBuffer.inSize = bufferVulkan->size;
			settingsStageBuffer.inFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			settingsStageBuffer.inStagingBuffer = true;
			settingsStageBuffer.inCreateMapped = false;
			settingsStageBuffer.inUsage = Usage::MEMORY_USAGE_CPU_ONLY;

			if (!CreateBuffer_Internal(settingsStageBuffer))
			{
				LOG_BE_ERROR("{0} Failed to create new vulkan staging buffer", Tag);
				return INVALID_HANDLE(BufferHandle);
			}

			//map memory
			void* data = nullptr;
			vmaMapMemory(m_VmaAllocator, settingsStageBuffer.outMemAlloc, &data);
			memcpy(data, desc->initData, desc->size);
			vmaUnmapMemory(m_VmaAllocator, settingsStageBuffer.outMemAlloc);

			if (!CopyBuffer(settingsStageBuffer.outBuffer, bufferVulkan->buffer, settingsStageBuffer.inSize))
			{
				LOG_BE_ERROR("{0} Failed to copy data from staging buffer", Tag);
				vmaDestroyBuffer(m_VmaAllocator, settingsStageBuffer.outBuffer,settingsStageBuffer.outMemAlloc);
				return INVALID_HANDLE(BufferHandle);
			}
			vmaDestroyBuffer(m_VmaAllocator, settingsStageBuffer.outBuffer, settingsStageBuffer.outMemAlloc);
		}

		BufferHandle h;
		h.handle = m_ResContainer.buffers.Add(buffer);
		return h;
	}

	bool VulkanDevice::CreateBuffer_Internal(BufferInternalSettings& settings) const
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = settings.inSize;
		bufferInfo.usage = settings.inFlags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo memory_info{};
		if (settings.inStagingBuffer == true)
		{
			memory_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}
		
		if (settings.inCreateMapped)
		{
			memory_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}
		memory_info.usage = ConvertMemoryUsage(settings.inUsage);

		VmaAllocationInfo allocation_info{};

		if (vmaCreateBuffer(m_VmaAllocator, &bufferInfo, &memory_info, &settings.outBuffer, &settings.outMemAlloc, nullptr)!= VK_SUCCESS)
		{
			return false;
		}
		return true;
	}

	void VulkanDevice::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout currentLayout, VkImageLayout targetLayout) const
	{
		VkCommandBuffer cmd = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = currentLayout;
		barrier.newLayout = targetLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			LOG_BE_ERROR("{0} Failed to find proper source and destinations settings for image transition layer.", Tag);
			return;
		}


		vkCmdPipelineBarrier(
			cmd,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndSingleTimeCommands(cmd);
	}


	void VulkanDevice::CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, U32 w, U32 h) const
	{
		VkCommandBuffer cmd = BeginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			w,
			h,
			1
		};

		vkCmdCopyBufferToImage(
			cmd,
			srcBuffer,
			dstImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		EndSingleTimeCommands(cmd);
	}


#pragma endregion

#pragma region Release

	bool VulkanDevice::Release(BufferHandle& buffer) 
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(buffer), "Invalid buffer passed to release.");
		Buffer* b = nullptr;
		m_ResContainer.buffers.Get(buffer.handle, &b);
		m_ResContainer.buffers.Remove(buffer.handle);
		delete b;
		INVALIDATE_HANDLE(buffer)
		return true;
	}

	bool VulkanDevice::Release(PipelineHandle& pipeline) 
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(pipeline), "Invalid pipeline passed to release.");
		Pipeline* p = nullptr;
		m_ResContainer.pipelines.Get(pipeline.handle, &p);
		m_ResContainer.pipelines.Remove(pipeline.handle);
		delete p;
		INVALIDATE_HANDLE(pipeline)
		return true;
	}

	bool VulkanDevice::Release(ShaderHandle& shader) 
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(shader), "Invalid shader passed to release.");
		Shader* s = nullptr;
		m_ResContainer.shaders.Get(shader.handle, &s);
		m_ResContainer.shaders.Remove(shader.handle);
		delete s;
		INVALIDATE_HANDLE(shader)
		return true;
	}

	bool VulkanDevice::Release(RenderPassHandle& rp) 
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(rp), "Invalid render pass passed to release.");
		RenderPass* r = nullptr;
		m_ResContainer.renderPasses.Get(rp.handle, &r);
		m_ResContainer.renderPasses.Remove(rp.handle);
		delete r;
		INVALIDATE_HANDLE(rp)
		return true;
	}

	bool VulkanDevice::Release(DescriptorSetLayoutHandle& dsl) 
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(dsl), "Invalid descriptor set layout to release");
		DescriptorSetLayout* d = nullptr;
		m_ResContainer.descriptorSetLayouts.Get(dsl.handle, &d);
		m_ResContainer.descriptorSetLayouts.Remove(dsl.handle);
		delete d;
		INVALIDATE_HANDLE(dsl)
		return true;
	}

	bool VulkanDevice::Release(FrameBufferHandle& fb) 
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(fb), "Invalid frame buffer to release");
		FrameBuffer* f = nullptr;
		m_ResContainer.frameBuffers.Get(fb.handle, &f);
		m_ResContainer.frameBuffers.Remove(fb.handle);
		delete f;
		INVALIDATE_HANDLE(fb)
		return true;
	}

	bool VulkanDevice::Release(CommandBufferHandle& cb)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(cb), "Invalid frame buffer to release");
		CommandBuffer* c = nullptr;
		m_ResContainer.commandBuffers.Get(cb.handle, &c);
		m_ResContainer.commandBuffers.Remove(cb.handle);
		c->Release();
		delete c;
		INVALIDATE_HANDLE(cb)
		return true;
	}

	bool VulkanDevice::Release(TextureHandle& texture) 
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(texture), "Invalid texture to release");
		Texture* t = nullptr;
		m_ResContainer.textures.Get(texture.handle, &t);
		m_ResContainer.textures.Remove(texture.handle);
		delete t;
		INVALIDATE_HANDLE(texture)
		return true;
	}

	DescriptorAllocator* VulkanDevice::CreateDescriptorAllocator()
	{
		auto allocator = new DescriptorAllocator();
		allocator->Init(this);
		return allocator;
	}

	void VulkanDevice::ReleaseDescriptorAllocator(DescriptorAllocator* allocator)
	{
		if (allocator != nullptr)
		{
			allocator->CleanUp();
			delete allocator;
		}
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

#pragma region helpers

	U32 VulkanDevice::FindMemoryType(U32 typeFilter, VkMemoryPropertyFlags properties) const
	{
		for (U32 i = 0; i < m_MemProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (m_MemProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		LOG_BE_ERROR("{0} Failed to find fitting memory type.", Tag);
	}

	VkDeviceMemory VulkanDevice::AllocateBufferMemory(VkBuffer buffer,VkMemoryPropertyFlags memoryProperty) const
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

		U32 memoryType = FindMemoryType(memRequirements.memoryTypeBits, memoryProperty);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = memoryType;

		VkDeviceMemory vertexBufferMemory;

		if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to allocate memory for a buffer.", Tag);
			return nullptr;
		}

		vkBindBufferMemory(m_Device, buffer, vertexBufferMemory, 0);

		return vertexBufferMemory;
	}

	VkSampler VulkanDevice::CreateImageSampler(const SamplerDesc& desc) const
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = ConvertFilterType(desc.magFilter);
		samplerInfo.minFilter = ConvertFilterType(desc.minFilter);
		samplerInfo.addressModeU = ConvertAddressMode(desc.addressModeU);
		samplerInfo.addressModeV = ConvertAddressMode(desc.addressModeV);
		samplerInfo.addressModeW = ConvertAddressMode(desc.addressModeW);
		samplerInfo.anisotropyEnable = desc.anisoEnabled;
		samplerInfo.maxAnisotropy = m_DeviceProperties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkSampler sampler;
		if (vkCreateSampler(m_Device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} failed to create texture sampler!", Tag);
			return nullptr;
		}

		return sampler;
	}

	VkImageView VulkanDevice::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const
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

	bool VulkanDevice::CreateAllocatorObject()
	{
		VmaAllocatorCreateInfo createInfo{};
		createInfo.device = m_Device;
		createInfo.instance = m_Instance;
		createInfo.physicalDevice = m_PhysicalDevice;

		if (vmaCreateAllocator(&createInfo, &m_VmaAllocator) != VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} Failed to create VMA allocator", Tag);
			return false;
		}

		return true;
	}

	void VulkanDevice::OnEvent(const Czuch::Event& e)
	{
		if (e.GetEventType() == WindowSizeChangedEvent::GetStaticEventType())
		{
			m_FrameBufferResized = true;
		}
	}

	void VulkanDevice::EndSingleTimeCommands(VkCommandBuffer cmd) const
	{
		vkEndCommandBuffer(cmd);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_GraphicsQueue);

		vkFreeCommandBuffers(m_Device, m_CopyCommandPool, 1, &cmd);
	}

	VkCommandBuffer VulkanDevice::BeginSingleTimeCommands() const
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_CopyCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	bool VulkanDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
	{
		auto cmd = BeginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(cmd);

		return true;
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

	void VulkanDevice::AwaitDevice() const
	{
		vkDeviceWaitIdle(m_Device);
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
		m_DescriptorLayoutCache->CleanUp();
		delete m_DescriptorLayoutCache;

		EventsManager::Get().RemoveListener(WindowSizeChangedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		m_ResContainer.ReleaseAll();

		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_CopyCommandPool, nullptr);
		m_SwapChainData.Release(m_Device);
		vmaDestroyAllocator(m_VmaAllocator);
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

		if (CreateAllocatorObject() == false)
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

		m_DescriptorLayoutCache = new DescriptorLayoutCache();
		m_DescriptorLayoutCache->Init(m_Device);

		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_DeviceProperties);

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

		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemProperties);

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

		RenderPass_Vulkan* rpass = Internal_to_RenderPass(AccessRenderPass(m_SwapChainRenderPass));

		for (size_t i = 0; i < m_SwapChainData.swapChainImageViews.size(); i++) {

			VkImageView attachments[] = {
				m_SwapChainData.swapChainImageViews[i]
			};

			FrameBuffer* framebuffer = new FrameBuffer();
			framebuffer->m_InternalResourceState = std::make_shared<FrameBuffer_Vulkan>();
			FrameBufferHandle h;
			h .handle= m_ResContainer.frameBuffers.Add(framebuffer);
			m_SwapChainData.swapChainFrameBuffers[i] = h;

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = rpass->renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_SwapChainData.swapChainExtent.width;
			framebufferInfo.height = m_SwapChainData.swapChainExtent.height;
			framebufferInfo.layers = 1;

			FrameBuffer_Vulkan* fbuffer = Internal_to_Framebuffer(framebuffer);

			fbuffer->createInfo = framebufferInfo;
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

		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CopyCommandPool) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to create command pool.", Tag);
			return false;
		}

		return true;
	}

	void VulkanDevice::ResourcesContainer::ReleaseAll()
	{
		shaders.RemoveAll();
		textures.RemoveAll();
		buffers.RemoveAll();
		commandBuffers.RemoveAll();
		descriptorSetLayouts.RemoveAll();
		pipelines.RemoveAll();
		renderPasses.RemoveAll();
		frameBuffers.RemoveAll();
	}

	Shader* VulkanDevice::AccessShader(ShaderHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			Shader* result = nullptr;
			m_ResContainer.shaders.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccessShader with invalid handle.", Tag);
		return nullptr;
	}

	CommandBuffer* VulkanDevice::AccessCommandBuffer(CommandBufferHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			CommandBuffer* result = nullptr;
			m_ResContainer.commandBuffers.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccessCommandBuffer with invalid handle.", Tag);
		return nullptr;
	}

	Buffer* VulkanDevice::AccessBuffer(BufferHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			Buffer* result = nullptr;
			m_ResContainer.buffers.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccessBuffer with invalid handle.", Tag);
		return nullptr;
	}

	Texture* VulkanDevice::AccessTexture(TextureHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			Texture* result = nullptr;
			m_ResContainer.textures.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccessTexture with invalid handle.", Tag);
		return nullptr;
	}

	Pipeline* VulkanDevice::AccessPipeline(PipelineHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			Pipeline* result = nullptr;
			m_ResContainer.pipelines.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccessPipeline with invalid handle.", Tag);
		return nullptr;
	}

	RenderPass* VulkanDevice::AccessRenderPass(RenderPassHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			RenderPass* result = nullptr;
			m_ResContainer.renderPasses.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccessRenderPass with invalid handle.", Tag);
		return nullptr;
	}

	FrameBuffer* VulkanDevice::AccessFrameBuffer(FrameBufferHandle handle) 
	{
		if (HANDLE_IS_VALID(handle))
		{
			FrameBuffer* result = nullptr;
			m_ResContainer.frameBuffers.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccesFrameBuffer with invalid handle.", Tag);
		return nullptr;
	}

	DescriptorSetLayout* VulkanDevice::AccessDescriptorSetLayout(DescriptorSetLayoutHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			DescriptorSetLayout* result = nullptr;
			m_ResContainer.descriptorSetLayouts.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccessDescriptorSetLayout with invalid handle.", Tag);
		return nullptr;
	}

}
