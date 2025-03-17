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
#include"./Subsystems/Assets/AssetsManager.h"
#include"./Subsystems/Assets/Asset/ShaderAsset.h"
#include"./Subsystems/Assets/Asset/MaterialAsset.h"
#include"./Subsystems/Assets/Asset/MaterialInstanceAsset.h"
#include "imgui.h"
#include"backends/imgui_impl_glfw.h"
#include"backends/imgui_impl_vulkan.h"
#include"Renderer/Renderer.h"
#include<set>


namespace Czuch
{
	const CzuchStr Tag = "[VulkanDevice]";

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
		VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
		VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
	};
	const std::vector<const char*> validationLayers = {
"VK_LAYER_KHRONOS_validation"
	};

#pragma region Create methods

	VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice, bool includeStencil);
	bool CreateImage(VmaAllocator allocator, TextureDesc::Type type, U32 width, U32 height, U32 depth, U32 mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memUsage,
		VkImage& image, VmaAllocation& alloc);
	bool HasStencilComponent(VkFormat format);
	VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	void VulkanDevice::DrawUI(CommandBuffer* commandBuffer)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)WindowInfo::Width, (float)WindowInfo::Height);

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<VulkanCommandBuffer*>(commandBuffer)->GetNativeBuffer());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void VulkanDevice::PreDrawFrame()
	{

	}

	void* VulkanDevice::CreatePointerForUITexture(TextureHandle tex)
	{
		auto texture = AccessTexture(tex);
		auto vulkanTexture = Internal_to_Texture(texture);
		VkDescriptorSet set = ImGui_ImplVulkan_AddTexture(vulkanTexture->sampler, vulkanTexture->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return set;
	}

	PipelineHandle VulkanDevice::CreatePipelineState(const MaterialPassDesc* desc, RenderPass* rpass, bool dynamicRendering)
	{
		CZUCH_BE_ASSERT(desc, "CreatePipelineState NULL desc input");

		RenderPass* rp = rpass;
		if (!dynamicRendering)
		{
			if (rpass==nullptr)
			{
				rp = AccessRenderPass(m_SwapChainRenderPass);
			}
		}

		if (rp == nullptr)
		{
			LOG_BE_ERROR("[{0}] Failed to create new pipeline state, render pass is null", Tag);
			return INVALID_HANDLE(PipelineHandle);
		}


		Pipeline* ps = new Pipeline();
		ps->m_InternalResourceState = std::make_shared<Pipeline_Vulkan>();
		ps->m_desc = std::move(*desc);
		ps->device = this;

		auto vsAsset = AssetsManager::GetPtr()->GetAsset<ShaderAsset>(desc->vs);
		auto psAsset = AssetsManager::GetPtr()->GetAsset<ShaderAsset>(desc->ps);

		ps->vs = vsAsset->GetShaderAssetHandle();
		ps->ps = psAsset->GetShaderAssetHandle();

		for (int a = 0; a < desc->layoutsCount; a++)
		{
			ps->AddLayout(CreateDescriptorSetLayout(&desc->layouts[a]));
		}

		//VkRenderPass renderPass = rp != nullptr ? Internal_to_RenderPass(rp)->renderPass : VK_NULL_HANDLE;

		VulkanPipelineBuilder builder(this, Internal_To_Pipeline(ps), ps);
		if (!builder.BuildPipeline(rp))
		{
			LOG_BE_ERROR("[{0}] Failed to Build new pipeline", Tag);
			delete ps;
			return INVALID_HANDLE(PipelineHandle);
		}

		PipelineHandle h;
		h.handle = m_ResContainer.pipelines.Add(ps);

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
			LOG_BE_ERROR("{0} Failed to create new shader module", Tag);
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

	VkAttachmentDescription colorAttachments[8] = {};
	VkAttachmentReference colorAttachmentsRef[8] = {};
	VkAttachmentDescription depthAttachment{};
	VkAttachmentReference depthAttachmentRef{};
	VkSubpassDescription subpass = {};
	VkAttachmentDescription attachments[k_max_image_outputs + 1]{};

	void FillRenderPassInfo(VkRenderPassCreateInfo& info, const RenderPassDesc& desc)
	{
		VkAttachmentLoadOp depthOp, stencilOp;
		VkImageLayout depthStart;

		switch (desc.depthLoadOp)
		{
		case AttachmentLoadOp::LOAD:
			depthOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthStart = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			break;
		case AttachmentLoadOp::CLEAR:
			depthOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthStart = VK_IMAGE_LAYOUT_UNDEFINED;
			break;
		default:
			depthOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthStart = VK_IMAGE_LAYOUT_UNDEFINED;
			break;
		}

		switch (desc.stencilLoadOp)
		{
		case AttachmentLoadOp::LOAD:
			stencilOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			break;
		case AttachmentLoadOp::CLEAR:
			stencilOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			break;
		default:
			stencilOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			break;
		}

		//color attachments
		for (U32 c = 0; c < desc.attachmentsCount; ++c) {
			VkAttachmentLoadOp colorOp;
			VkImageLayout colorInitial;

			switch (desc.colorAttachments[c].loadOp)
			{
			case AttachmentLoadOp::LOAD:
				colorOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				colorInitial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case AttachmentLoadOp::CLEAR:
				colorOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorInitial = VK_IMAGE_LAYOUT_UNDEFINED;
				break;
			default:
				colorOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorInitial = VK_IMAGE_LAYOUT_UNDEFINED;
				break;
			}

			VkAttachmentDescription& color_attachment = colorAttachments[c];
			color_attachment.format = ConvertFormat(desc.colorAttachments[c].format);
			color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			color_attachment.loadOp = colorOp;
			color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			color_attachment.stencilLoadOp = stencilOp;
			color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			color_attachment.initialLayout = colorInitial;
			color_attachment.finalLayout = ConvertImageLayout(desc.colorAttachments[c].layout);
			color_attachment.flags = 0;

			VkAttachmentReference& color_attachment_ref = colorAttachmentsRef[c];
			color_attachment_ref.attachment = c;
			color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		// Depth attachment
		if (IsDepthFormat(desc.depthStencilFormat)) {

			depthAttachment.format = ConvertFormat(desc.depthStencilFormat);
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = depthOp;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = stencilOp;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = depthStart;
			depthAttachment.finalLayout = ConvertImageLayout(desc.depthStencilFinalLayout);
			depthAttachment.flags = 0;

			depthAttachmentRef.attachment = desc.attachmentsCount;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		for (U32 activeAttachments = 0; activeAttachments < desc.attachmentsCount; ++activeAttachments) {
			attachments[activeAttachments] = colorAttachments[activeAttachments];
		}
		subpass.colorAttachmentCount = desc.attachmentsCount;
		subpass.pColorAttachments = desc.attachmentsCount==0?nullptr:colorAttachmentsRef;

		subpass.pDepthStencilAttachment = nullptr;

		U32 depthStencilCount = 0;
		if (IsDepthFormat(desc.depthStencilFormat))
		{
			attachments[subpass.colorAttachmentCount] = depthAttachment;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			depthStencilCount = 1;
		}

		info.attachmentCount = desc.attachmentsCount + depthStencilCount;
		info.pAttachments = attachments;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
	}

	RenderPassHandle VulkanDevice::CreateRenderPass(const RenderPassDesc* desc)
	{
		RenderPass* rp = new RenderPass();
		rp->m_InternalResourceState = std::make_shared<RenderPass_Vulkan>();
		if (desc != nullptr)
		{
			rp->desc = *desc;
		}

		if (m_RenderSettings->dynamicRendering)
		{
			RenderPassHandle h;
			h.handle = m_ResContainer.renderPasses.Add(rp);
			return h;
		}

		bool mainPass = desc == nullptr;
		const VulkanRenderPassDesc* m_RpDesc = !mainPass ? dynamic_cast<const VulkanRenderPassDesc*>(desc) : nullptr;

		if (m_RpDesc != nullptr && m_RpDesc->mainRenderPass)
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = m_SwapChainData.swapChainImageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = FindDepthFormat(m_PhysicalDevice, true);
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			VkAttachmentDescription attachmentsArray[2] = { colorAttachment,depthAttachment };

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 2;
			renderPassInfo.pAttachments = attachmentsArray;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			RenderPass_Vulkan* rpass = Internal_to_RenderPass(rp);
			rpass->device = m_Device;

			if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &(rpass->renderPass)) != VK_SUCCESS) {
				LOG_BE_ERROR("{0} Failed to create new render pass", Tag);
				delete rp;
				return INVALID_HANDLE(RenderPassHandle);
			}

			RenderPassHandle h;
			h.handle = m_ResContainer.renderPasses.Add(rp);
			return h;
		}


		RenderPass_Vulkan* rpass = Internal_to_RenderPass(rp);
		rpass->device = m_Device;

		if (m_RpDesc != nullptr)
		{
			if (vkCreateRenderPass(m_Device, &m_RpDesc->renderPassInfo, nullptr, &(rpass->renderPass)) != VK_SUCCESS) {
				LOG_BE_ERROR("{0} Failed to create new render pass", Tag);
				delete rp;
				return INVALID_HANDLE(RenderPassHandle);
			}

		}
		else
		{
			//create vk render pass info from base struct
			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			FillRenderPassInfo(renderPassInfo, rp->desc);
			if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &(rpass->renderPass)) != VK_SUCCESS) {
				LOG_BE_ERROR("{0} Failed to create new render pass", Tag);
				delete rp;
				return INVALID_HANDLE(RenderPassHandle);
			}
		}

		RenderPassHandle h;
		h.handle = m_ResContainer.renderPasses.Add(rp);

		return h;
	}

	VkDescriptorSetLayoutBinding CreateBinding(const DescriptorSetLayoutDesc::Binding& binding, U32 stages)
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
			bindingsArray[a] = CreateBinding(desc->bindings[a], desc->shaderStage);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = desc->bindingsCount;
		layoutInfo.pBindings = bindingsArray;

		DescriptorSetLayout_Vulkan* dslayout = Internal_to_DescriptorSetLayout(dsl);
		dslayout->device = m_Device;

		dslayout->layout = m_DescriptorLayoutCache->CreateDescriptorLayout(&layoutInfo);

		if (dslayout->layout == nullptr) {
			LOG_BE_ERROR("{0} Failed to create new descriptor set layout", Tag);
			delete dsl;
			return INVALID_HANDLE(DescriptorSetLayoutHandle);
		}


		DescriptorSetLayoutHandle h;
		h.handle = m_ResContainer.descriptorSetLayouts.Add(dsl);
		return h;
	}

	FrameBufferHandle VulkanDevice::CreateFrameBuffer(const FrameBufferDesc* desc, bool resize, FrameBufferHandle handle)
	{
		CZUCH_BE_ASSERT(desc != nullptr, "Invalid frame buffer desc.");

		if (resize)
		{
			FrameBuffer* fb = AccessFrameBuffer(handle);
			if (HasDynamicRenderingEnabled())
			{
				return handle;
			}
			FrameBuffer_Vulkan* vulkanFramebuffer = Internal_to_Framebuffer(fb);
			vulkanFramebuffer->Release();

			VkImageView attachments[k_max_image_outputs];
			U32 count = HANDLE_IS_VALID(desc->depthStencilTexture) ? desc->renderTargetsCount + 1 : desc->renderTargetsCount;

			for (U32 a = 0; a < desc->renderTargetsCount; a++)
			{
				Texture* colorTexture = AccessTexture(desc->renderTextures[a]);
				attachments[a] = Internal_to_Texture(colorTexture)->imageView;
			}

			if (HANDLE_IS_VALID(desc->depthStencilTexture))
			{
				Texture* depthTexture = AccessTexture(desc->depthStencilTexture);
				attachments[desc->renderTargetsCount] = Internal_to_Texture(depthTexture)->imageView;
			}

			vulkanFramebuffer->createInfo.pAttachments = attachments;
			vulkanFramebuffer->createInfo.width = desc->width;
			vulkanFramebuffer->createInfo.height = desc->height;

			if (vkCreateFramebuffer(m_Device, &vulkanFramebuffer->createInfo, nullptr, &vulkanFramebuffer->framebuffer) != VK_SUCCESS) {
				LOG_BE_ERROR("{0} Failed to create new frame buffer", Tag);
				return INVALID_HANDLE(FrameBufferHandle);
			}

			return handle;
		}

		FrameBuffer* fb = new FrameBuffer();
		fb->m_InternalResourceState = std::make_shared<FrameBuffer_Vulkan>();
		fb->desc = *desc;

		if (HasDynamicRenderingEnabled())
		{
			FrameBufferHandle h;
			h.handle = m_ResContainer.frameBuffers.Add(fb);
			return h;
		}

		RenderPass* rp = AccessRenderPass(desc->renderPass);

		VkImageView attachments[k_max_image_outputs];
		U32 count = HANDLE_IS_VALID(desc->depthStencilTexture) ? desc->renderTargetsCount + 1 : desc->renderTargetsCount;

		for (U32 a = 0; a < desc->renderTargetsCount; a++)
		{
			Texture* colorTexture = AccessTexture(desc->renderTextures[a]);
			attachments[a] = Internal_to_Texture(colorTexture)->imageView;
		}

		if (HANDLE_IS_VALID(desc->depthStencilTexture))
		{
			Texture* depthTexture = AccessTexture(desc->depthStencilTexture);
			attachments[desc->renderTargetsCount] = Internal_to_Texture(depthTexture)->imageView;
		}

		CZUCH_BE_ASSERT(rp != nullptr, "Invalid frame buffer desc's render pass.");

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = Internal_to_RenderPass(rp)->renderPass;
		framebufferInfo.attachmentCount = count;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = desc->width;
		framebufferInfo.height = desc->height;
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

	CommandBufferHandle VulkanDevice::CreateCommandBuffer(bool isPrimary, void* pool)
	{
		VulkanCommandBuffer* cmdBuffer = nullptr;
		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = pool == nullptr ? m_CommandPool : (VkCommandPool)pool;;
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

	void VulkanDevice::GenerateMipmaps(VkImage image, VkFormat imageFormat, U32 texWidth, U32 texHeight, U32 mipLevels)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		I32 mipWidth = texWidth;
		I32 mipHeight = texHeight;

		for (U32 i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommands(commandBuffer);
	}

	TextureHandle VulkanDevice::CreateTexture(const TextureDesc* desc, bool resize, TextureHandle handle)
	{
		CZUCH_BE_ASSERT(desc != nullptr, "Invalid texture desc.");

		Texture* color_texture = nullptr;
		Texture_Vulkan* vulkanTexture = nullptr;

		if (resize == false)
		{
			color_texture = new Texture();
			color_texture->desc = *desc;
			color_texture->m_InternalResourceState = std::make_shared<Texture_Vulkan>();

			vulkanTexture = Internal_to_Texture(color_texture);
			vulkanTexture->allocator = m_VmaAllocator;
			vulkanTexture->device = m_Device;
		}
		else
		{
			color_texture = AccessTexture(handle);
			vulkanTexture = Internal_to_Texture(color_texture);
			vulkanTexture->Release();
		}


		if (desc->texData != nullptr)
		{

			BufferInternalSettings settingsStageBuffer{};
			settingsStageBuffer.inSize = desc->GetSize();
			settingsStageBuffer.inFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			settingsStageBuffer.inStagingBuffer = true;
			settingsStageBuffer.inCreateMapped = false;
			settingsStageBuffer.inUsage = Usage::MEMORY_USAGE_CPU_ONLY;

			if (!CreateBuffer_Internal(settingsStageBuffer))
			{
				LOG_BE_ERROR("{0} Failed to create staging buffer for texture copy with name: {1}.", Tag, desc->name);
				return INVALID_HANDLE(TextureHandle);
			}

			void* data;
			vmaMapMemory(m_VmaAllocator, settingsStageBuffer.outMemAlloc, &data);
			memcpy(data, desc->texData, settingsStageBuffer.inSize);
			vmaUnmapMemory(m_VmaAllocator, settingsStageBuffer.outMemAlloc);
			VkFormat targetFormat = ConvertFormat(desc->format);

			auto imageWithAlloc = CreateImage(desc->type, desc->width, desc->height, desc->mip_levels, ConvertSamplesCount(desc->sample_count), targetFormat,
				VK_IMAGE_TILING_OPTIMAL,/* VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT*/ ConvertImageUsageFlags((U32)desc->usageFlags), VK_IMAGE_LAYOUT_UNDEFINED, VK_SHARING_MODE_EXCLUSIVE);

			if (imageWithAlloc.allocation == nullptr || imageWithAlloc.image == nullptr)
			{
				LOG_BE_ERROR("{0} Failed to create new vulkan Texture with name: {1}", Tag, desc->name);
				return INVALID_HANDLE(TextureHandle);
			}

			vulkanTexture->image = imageWithAlloc.image;
			vulkanTexture->allocation = imageWithAlloc.allocation;

			//transition
			TransitionImageLayout(nullptr,vulkanTexture->image, targetFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,0,desc->mip_levels,IsDepthFormat(desc->format));

			//copy staging buffe to target image
			CopyBufferToImage(settingsStageBuffer.outBuffer, vulkanTexture->image, desc->width, desc->height);

			if (desc->mip_levels > 1)
			{
				GenerateMipmaps(vulkanTexture->image, targetFormat, desc->width, desc->height, desc->mip_levels);
			}
			else
			{
				//transition for read in ps
				TransitionImageLayout(nullptr, vulkanTexture->image, targetFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ConvertImageLayout(ImageLayout::SHADER_READ_ONLY_OPTIMAL), 0, desc->mip_levels, IsDepthFormat(desc->format));
			}
	

			vmaDestroyBuffer(m_VmaAllocator, settingsStageBuffer.outBuffer, settingsStageBuffer.outMemAlloc);
		}
		else
		{
			VkFormat targetFormat = ConvertFormat(desc->format);
			auto imageWithAlloc = CreateImage(desc->type, desc->width, desc->height, desc->mip_levels, ConvertSamplesCount(desc->sample_count), targetFormat,
				VK_IMAGE_TILING_OPTIMAL, ConvertImageUsageFlags((U32)desc->usageFlags), VK_IMAGE_LAYOUT_UNDEFINED, VK_SHARING_MODE_EXCLUSIVE);

			if (imageWithAlloc.allocation == nullptr || imageWithAlloc.image == nullptr)
			{
				LOG_BE_ERROR("{0} Failed to create new vulkan Texture with name: {1}", Tag, desc->name);
				return INVALID_HANDLE(TextureHandle);
			}

			vulkanTexture->image = imageWithAlloc.image;
			vulkanTexture->allocation = imageWithAlloc.allocation;

			if (desc->resourceType & ResourceState::SHADER_RESOURCE && desc->mip_levels<=1)
			{
				TransitionImageLayout(nullptr,vulkanTexture->image, targetFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,0,desc->mip_levels,IsDepthFormat(desc->format));
			}
			else if(desc->mip_levels > 1)
			{
				GenerateMipmaps(vulkanTexture->image, targetFormat, desc->width, desc->height, desc->mip_levels);
			}
		}

		auto aspect = ConvertImageAspect((U32)desc->aspectFlags);

		vulkanTexture->imageView = CreateImageView(vulkanTexture->image, ConvertFormat(desc->format), aspect,desc->mip_levels);
		vulkanTexture->sampler = CreateImageSampler(desc->samplerDesc);

		if (resize)
		{
			return handle;
		}

		TextureHandle h;
		h.handle = m_ResContainer.textures.Add(color_texture);
		LOG_BE_INFO("{0} Texture created with name: {1} with id: {2}",Tag,desc->name, h.handle);
		return h;
	}


	MeshHandle VulkanDevice::CreateMesh(MeshData& meshData)
	{
		Mesh* mesh = new Mesh();
		mesh->data = &meshData;
		mesh->device = this;
		MaterialInstanceHandle matInstanceHandle = AssetsManager::GetPtr()->GetAsset<MaterialInstanceAsset>(meshData.materialInstanceAssetHandle)->GetMaterialInstanceResourceHandle();
		mesh->materialHandle = matInstanceHandle;

		BufferDesc vbDesc;
		vbDesc.elementsCount = mesh->data->positions.size();
		vbDesc.size = vbDesc.elementsCount * sizeof(float) * 3;
		vbDesc.stride = 3 * sizeof(float);
		vbDesc.usage = Usage::DEFAULT;
		vbDesc.bind_flags = BindFlag::VERTEX_BUFFER;
		vbDesc.initData = (void*)mesh->data->positions.data();

		mesh->positionsHandle = CreateBuffer(&vbDesc);

		if (mesh->HasNormals())
		{
			BufferDesc nDesc;
			nDesc.elementsCount = mesh->data->normals.size();
			nDesc.size = nDesc.elementsCount * sizeof(float) * 3;
			nDesc.stride = 3 * sizeof(float);
			nDesc.usage = Usage::DEFAULT;
			nDesc.bind_flags = BindFlag::VERTEX_BUFFER;
			nDesc.initData = (void*)mesh->data->normals.data();

			mesh->normalsHandle = CreateBuffer(&nDesc);
		}

		if (mesh->HasColors())
		{
			BufferDesc cDesc;
			cDesc.elementsCount = mesh->data->colors.size();
			cDesc.size = cDesc.elementsCount * sizeof(float) * 4;
			cDesc.stride = 4 * sizeof(float);
			cDesc.usage = Usage::DEFAULT;
			cDesc.bind_flags = BindFlag::VERTEX_BUFFER;
			cDesc.initData = (void*)mesh->data->colors.data();

			mesh->colorsHandle = CreateBuffer(&cDesc);
		}

		if (mesh->HasUV0())
		{
			BufferDesc uvDesc;
			uvDesc.elementsCount = mesh->data->uvs0.size();
			uvDesc.size = uvDesc.elementsCount * sizeof(float) * 4;
			uvDesc.stride = 4 * sizeof(float);
			uvDesc.usage = Usage::DEFAULT;
			uvDesc.bind_flags = BindFlag::VERTEX_BUFFER;
			uvDesc.initData = (void*)mesh->data->uvs0.data();

			mesh->uvs0Handle = CreateBuffer(&uvDesc);
		}

		BufferDesc indicesDesc;
		indicesDesc.elementsCount = mesh->data->indices.size();
		indicesDesc.size = indicesDesc.elementsCount * sizeof(U32);
		indicesDesc.stride = sizeof(U32);
		indicesDesc.usage = Usage::DEFAULT;
		indicesDesc.bind_flags = BindFlag::INDEX_BUFFER;
		indicesDesc.initData = (void*)mesh->data->indices.data();

		mesh->indicesHandle = CreateBuffer(&indicesDesc);

		MeshHandle h;
		h.handle = m_ResContainer.meshes.Add(mesh);
		return h;
	}

	MaterialHandle VulkanDevice::CreateMaterial(MaterialDefinitionDesc* materialData)
	{
		U32 passesCount = materialData->PassesCount();
		Material* material = new Material();
		material->desc =materialData;
		material->pipelines.resize(passesCount);
		for (U32 a = 0; a < passesCount; a++)
		{
			auto& passDesc = material->GetDesc().GetMaterialPassDescAt(a);
			material->pipelines[a] = CreatePipelineState(&passDesc, GetRenderPassOfType(passDesc.passType), m_RenderSettings->dynamicRendering);
		}

		MaterialHandle h;
		h.handle = m_ResContainer.materials.Add(material);
		return h;
	}

	MaterialInstanceHandle VulkanDevice::CreateMaterialInstance(MaterialInstanceDesc& materialInstanceDesc)
	{
		MaterialInstance* matInstance = new MaterialInstance();
		matInstance->desc = &materialInstanceDesc;
		auto asset = AssetsManager::GetPtr()->GetAsset<MaterialAsset>(matInstance->desc->materialAsset);
		matInstance->handle = asset->GetMaterialResourceHandle();
		matInstance->desc->isTransparent = asset->IsTransparent();
		matInstance->passesCount = asset->GetPassesCount();
		auto material = AccessMaterial(matInstance->handle);
		auto* matDesc = material->desc;

		//make sure all buffers are created
		auto desc= &materialInstanceDesc;
		for (int i = 0; i < desc->paramsDesc.size(); ++i)
		{
			auto& paramDesc = desc->paramsDesc[i];
			if (paramDesc.type == DescriptorType::UNIFORM_BUFFER && paramDesc.isInternal==false)
			{
				if (paramDesc.resource == Invalid_Handle_Id)
				{
					auto buffer = CreateUBOBuffer(&paramDesc.uboData);
					paramDesc.resource = buffer.handle;
				}
			}
		}


		for (int a = 0; a < material->pipelines.size(); a++)
		{
			matDesc->passesContainer.states[a].SetParams(*matInstance->desc, matInstance->params[a]);
		}

		MaterialInstanceHandle h;
		h.handle = m_ResContainer.materialInstances.Add(matInstance);
		return h;
	}

	BufferHandle VulkanDevice::CreateUBOBuffer(UBO* ubo)
	{
		BufferDesc bufferDesc{};
		bufferDesc.size = ubo->GetSize();
		bufferDesc.usage = Usage::MEMORY_USAGE_GPU_ONLY;
		bufferDesc.stride = ubo->GetSize();
		bufferDesc.elementsCount = 1;
		bufferDesc.bind_flags = BindFlag::UNIFORM_BUFFER;
		bufferDesc.initData =ubo->GetData();

		bufferDesc.ubo = ubo;
		return CreateBuffer(&bufferDesc);
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
			usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		}
		if (HasFlag(buffer->desc.bind_flags, BindFlag::INDEX_BUFFER))
		{
			usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		if (HasFlag(buffer->desc.bind_flags, BindFlag::CONSTANT_BUFFER))
		{
			usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		if (HasFlag(buffer->desc.bind_flags, BindFlag::SHADER_RESOURCE))
		{
			usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // read only ByteAddressBuffer is also storage buffer
			usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		}
		if (HasFlag(buffer->desc.bind_flags, BindFlag::UNORDERED_ACCESS))
		{
			usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
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


		if ((HasFlag(desc->bind_flags, BindFlag::VERTEX_BUFFER) || HasFlag(desc->bind_flags, BindFlag::INDEX_BUFFER)) && desc->initData != nullptr)
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
				vmaDestroyBuffer(m_VmaAllocator, settingsStageBuffer.outBuffer, settingsStageBuffer.outMemAlloc);
				return INVALID_HANDLE(BufferHandle);
			}
			vmaDestroyBuffer(m_VmaAllocator, settingsStageBuffer.outBuffer, settingsStageBuffer.outMemAlloc);
		}
		else if (HasFlag(desc->bind_flags, BindFlag::UNIFORM_BUFFER) && desc->createMapped==false && desc->initData!=nullptr)
		{
			BufferInternalSettings settingsStageBuffer{};
			settingsStageBuffer.inSize = bufferVulkan->size;
			settingsStageBuffer.inFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			settingsStageBuffer.inStagingBuffer = true;
			settingsStageBuffer.inCreateMapped = false;
			settingsStageBuffer.inUsage = Usage::MEMORY_USAGE_CPU_ONLY;

			if (!CreateBuffer_Internal(settingsStageBuffer))
			{
				LOG_BE_ERROR("{0} Failed to create new vulkan staging buffer for uniform buffer transfer", Tag);
				return INVALID_HANDLE(BufferHandle);
			}

			//map memory
			void* data = nullptr;
			vmaMapMemory(m_VmaAllocator, settingsStageBuffer.outMemAlloc, &data);
			memcpy(data, desc->initData, desc->size);
			vmaUnmapMemory(m_VmaAllocator, settingsStageBuffer.outMemAlloc);

			if (!CopyBuffer(settingsStageBuffer.outBuffer, bufferVulkan->buffer, settingsStageBuffer.inSize))
			{
				LOG_BE_ERROR("{0} Failed to copy data from staging buffer to uniform buffer", Tag);
				vmaDestroyBuffer(m_VmaAllocator, settingsStageBuffer.outBuffer, settingsStageBuffer.outMemAlloc);
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

		if (vmaCreateBuffer(m_VmaAllocator, &bufferInfo, &memory_info, &settings.outBuffer, &settings.outMemAlloc, nullptr) != VK_SUCCESS)
		{
			return false;
		}
		return true;
	}

	void VulkanDevice::TransitionImageLayout(VkCommandBuffer cmd ,VkImage image, VkFormat format, VkImageLayout currentLayout, VkImageLayout targetLayout,U32 baseMipLevel,U32 mipCount,bool isDepth)const
	{
		bool singleTime = false;
		if (cmd == nullptr)
		{
			cmd = BeginSingleTimeCommands();
			singleTime = true;
		}

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = currentLayout;
		barrier.newLayout = targetLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		if (isDepth) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (HasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.image = image;
		barrier.subresourceRange.baseMipLevel = baseMipLevel;
		barrier.subresourceRange.levelCount = mipCount;
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
		else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask =  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else {
			LOG_BE_ERROR("{0} Failed to find proper source and destinations settings for image transition layer. From {1} to {2}", Tag, VulkanImageLayoutToString(currentLayout), VulkanImageLayoutToString(targetLayout));
			if (singleTime)
			{
				EndSingleTimeCommands(cmd);
			}
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

		if (singleTime)
		{
			EndSingleTimeCommands(cmd);
		}
	}

	void VulkanDevice::DoImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange) const
	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
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
		bool result = m_ResContainer.buffers.Get(buffer.handle, &b);
		m_ResContainer.buffers.Remove(buffer.handle);
		INVALIDATE_HANDLE(buffer)
		return true;
	}

	bool VulkanDevice::Release(PipelineHandle& pipeline)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(pipeline), "Invalid pipeline passed to release.");
		Pipeline* p = nullptr;
		m_ResContainer.pipelines.Get(pipeline.handle, &p);
		m_ResContainer.pipelines.Remove(pipeline.handle);
		INVALIDATE_HANDLE(pipeline)
			return true;
	}

	bool VulkanDevice::Release(ShaderHandle& shader)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(shader), "Invalid shader passed to release.");
		Shader* s = nullptr;
		m_ResContainer.shaders.Get(shader.handle, &s);
		m_ResContainer.shaders.Remove(shader.handle);
		INVALIDATE_HANDLE(shader)
			return true;
	}

	bool VulkanDevice::Release(RenderPassHandle& rp)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(rp), "Invalid render pass passed to release.");
		RenderPass* r = nullptr;
		m_ResContainer.renderPasses.Get(rp.handle, &r);
		m_ResContainer.renderPasses.Remove(rp.handle);
		INVALIDATE_HANDLE(rp)
			return true;
	}

	bool VulkanDevice::Release(DescriptorSetLayoutHandle& dsl)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(dsl), "Invalid descriptor set layout to release");
		DescriptorSetLayout* d = nullptr;
		m_ResContainer.descriptorSetLayouts.Get(dsl.handle, &d);
		m_ResContainer.descriptorSetLayouts.Remove(dsl.handle);
		INVALIDATE_HANDLE(dsl)
			return true;
	}

	bool VulkanDevice::Release(FrameBufferHandle& fb)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(fb), "Invalid frame buffer to release");
		FrameBuffer* f = nullptr;
		m_ResContainer.frameBuffers.Get(fb.handle, &f);
		m_ResContainer.frameBuffers.Remove(fb.handle);
		INVALIDATE_HANDLE(fb)
			return true;
	}

	bool VulkanDevice::Release(CommandBufferHandle& cb)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(cb), "Invalid frame buffer to release");
		CommandBuffer* c = nullptr;
		m_ResContainer.commandBuffers.Get(cb.handle, &c);
		c->Release();
		m_ResContainer.commandBuffers.Remove(cb.handle);
		INVALIDATE_HANDLE(cb)
			return true;
	}

	bool VulkanDevice::Release(TextureHandle& color_texture)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(color_texture), "Invalid texture to release");
		Texture* t = nullptr;
		m_ResContainer.textures.Get(color_texture.handle, &t);
		m_ResContainer.textures.Remove(color_texture.handle);
		INVALIDATE_HANDLE(color_texture)
			return true;
	}

	bool VulkanDevice::Release(MeshHandle& mesh)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(mesh), "Invalid mesh to release");
		Mesh* m = nullptr;
		m_ResContainer.meshes.Get(mesh.handle, &m);
		m_ResContainer.meshes.Remove(mesh.handle);
		INVALIDATE_HANDLE(mesh)
			return true;
	}

	bool VulkanDevice::Release(MaterialHandle& material)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(material), "Invalid material to release");
		Material* m = nullptr;
		m_ResContainer.materials.Get(material.handle, &m);
		m_ResContainer.materials.Remove(material.handle);
		INVALIDATE_HANDLE(material)
			return true;
	}

	bool VulkanDevice::Release(MaterialInstanceHandle& materialInstance)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(materialInstance), "Invalid material instance to release");
		MaterialInstance* m = nullptr;
		m_ResContainer.materialInstances.Get(materialInstance.handle, &m);
		m_ResContainer.materialInstances.Remove(materialInstance.handle);
		INVALIDATE_HANDLE(materialInstance)
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

	VkCommandPool VulkanDevice::CreateCommandPool(bool isTransient, bool isResettable)
	{
		QueueFamiliesIndexes queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, m_Surface);
		VkCommandPool commandPool;

		VkCommandPoolCreateFlags flags = 0;
		if (isTransient)
		{
			flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		}

		if (isResettable)
		{
			flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		}

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = flags;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to create command pool.", Tag);
			return nullptr;
		}
		return commandPool;
	}

	VkCommandBufferSubmitInfo VulkanDevice::CreateCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer)
	{
		VkCommandBufferSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		info.pNext = nullptr;
		info.commandBuffer = cmdBuffer;
		info.deviceMask = 0;

		return info;
	}

	VkSubmitInfo2 VulkanDevice::CreateSubmitInfo(VkCommandBufferSubmitInfo* cmdBufferSubmitInfo, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo)
	{
		VkSubmitInfo2 info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		info.pNext = nullptr;

		info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
		info.pWaitSemaphoreInfos = waitSemaphoreInfo;

		info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
		info.pSignalSemaphoreInfos = signalSemaphoreInfo;

		info.commandBufferInfoCount = 1;
		info.pCommandBufferInfos = cmdBufferSubmitInfo;

		return info;
	}

	void VulkanDevice::ReleaseCommandPool(VkCommandPool pool)
	{
		vkDestroyCommandPool(m_Device, pool, nullptr);
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
		VkResult result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FrameBufferResized) {
			m_FrameBufferResized = false;
			RecreateSwapChain();
		}
	}

	void VulkanDevice::BindSwapChainRenderPass(CommandBuffer* cmdBuffer, uint32_t imageIndex)
	{
		cmdBuffer->BindPass(m_SwapChainRenderPass, m_SwapChainData.swapChainFrameBuffers[imageIndex]);
	}

	void VulkanDevice::StartDynamicRenderPass(VulkanCommandBuffer* cmdBuffer, uint32_t imageIndex)
	{
		cmdBuffer->BeginDynamicRenderPassForMainPass(m_SwapChainData.swapChainImageViews[imageIndex], m_DepthImage.depthImageView, m_SwapChainData.swapChainExtent.width, m_SwapChainData.swapChainExtent.height);
	}

	bool VulkanDevice::HasDynamicRenderingEnabled() const
	{
		return m_RenderSettings->dynamicRendering;
	}

	void VulkanDevice::SubmitToGraphicsQueue(VkSubmitInfo info, VkFence fence)
	{
		VkResult result = vkQueueSubmit(m_GraphicsQueue, 1, &info, fence);
		if (result != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to submit to graphics queue due: {1}", Tag,VkResultToString(result));
		}
	}

	void VulkanDevice::ImmediateSubmitToGraphicsQueueWithCommandBuffer(VkCommandBuffer cmdBuffer, VkFence fence)
	{
		VkCommandBufferSubmitInfo cmdInfo = CreateCommandBufferSubmitInfo(cmdBuffer);
		VkSubmitInfo2 info = CreateSubmitInfo(&cmdInfo, nullptr, nullptr);
		if (vkQueueSubmit2(m_GraphicsQueue, 1, &info, fence) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to  immediate submit to graphics queue", Tag);
		}

		if (vkWaitForFences(m_Device, 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} Failed to wait for fence", Tag);
		}
	}

	uint32_t VulkanDevice::AcquireNextSwapChainImage(VkSemaphore sem, bool& aquireFailed)
	{
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_Device, m_SwapChainData.swapChain, UINT64_MAX, sem, VK_NULL_HANDLE, &imageIndex);

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
		m_CurrentImageIndex = imageIndex;
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

	VkDeviceMemory VulkanDevice::AllocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags memoryProperty) const
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
		samplerInfo.minLod = desc.minMipLevel;
		samplerInfo.maxLod = desc.maxMipLevel;

		VkSampler sampler;
		if (vkCreateSampler(m_Device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} failed to create texture sampler!", Tag);
			return nullptr;
		}

		return sampler;
	}

	VkImageView VulkanDevice::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, int mipsCount) const
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipsCount;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;

		if (vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} failed to create texture image view!", Tag);
			return nullptr;
		}
		return imageView;
	}

	ImageWithAllocation VulkanDevice::CreateImage(TextureDesc::Type type, U32 width, U32 height, U32 mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageLayout initLayout, VkSharingMode sharingMode) const
	{
		VkImageCreateInfo image{};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.imageType = ConvertImageType(type);
		image.format = format;
		image.extent.width = width;
		image.extent.height = height;
		image.extent.depth = 1;
		image.mipLevels = mipLevels;
		image.arrayLayers = 1;
		image.samples = numSamples;
		image.tiling = tiling;
		image.usage = usage;
		image.initialLayout = initLayout;
		image.sharingMode = sharingMode;
		image.flags = 0;

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocCreateInfo.priority = 1.0f;

		VkImage img;
		VmaAllocation alloc;

		VkImage textureImage;
		if (vmaCreateImage(m_VmaAllocator, &image, &allocCreateInfo, &img, &alloc, nullptr) != VK_SUCCESS) {
			LOG_BE_ERROR("{0} failed to create image!", Tag);
			return { nullptr,nullptr };
		}

		return { img,alloc };
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
#pragma region helpers

	bool CreateImage(VmaAllocator allocator, TextureDesc::Type type, U32 width, U32 height, U32 depth, U32 mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memUsage,
		VkImage& image, VmaAllocation& alloc)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = ConvertImageType(type);
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = depth;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VmaAllocationCreateInfo memory_info{};
		memory_info.usage = memUsage;

		if (vmaCreateImage(allocator, &imageInfo, &memory_info, &image, &alloc, nullptr) != VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} Failed to create new vulkan image", Tag);
			return false;
		}

		return true;
	}

	bool HasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
			return VkFormat::VK_FORMAT_UNDEFINED;
		}
	}


	VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice, bool includeStencil)
	{
		if (includeStencil)
		{
			return FindSupportedFormat(physicalDevice,
				{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			);
		}

		return FindSupportedFormat(physicalDevice,
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
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

#pragma endregion

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

	void VulkanDevice::OnEvent(Czuch::Event& e)
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
		m_DepthImage.Release(m_Device, m_VmaAllocator);

		CreateSwapChain();
		CreateDepthImage();
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

		if (m_RenderSettings->validationMode == ValidationMode::Enabled)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

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


	VulkanDevice::VulkanDevice(Window* window) :m_AttachedWindow(window), m_FrameBufferResized(false)
	{

	}


	VulkanDevice::~VulkanDevice()
	{
		m_DescriptorLayoutCache->CleanUp();
		delete m_DescriptorLayoutCache;

		EventsManager::Get().RemoveListener(WindowSizeChangedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		m_ResContainer.ReleaseAll();

		ReleaseCommandPool(m_CommandPool);
		ReleaseCommandPool(m_CopyCommandPool);
		m_DepthImage.Release(m_Device, m_VmaAllocator);
		m_SwapChainData.Release(m_Device);
		vmaDestroyAllocator(m_VmaAllocator);
		vkDestroyDevice(m_Device, nullptr);
		if (m_Surface != nullptr)
		{
			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		}

		vkDestroyInstance(m_Instance, nullptr);
	}


	bool VulkanDevice::InitDevice(RenderSettings* settings)
	{
		m_RenderSettings = settings;

		if (CreateVulkanInstance() == false)
		{
			return false;
		}
		else
		{
			LOG_BE_INFO("[VulkanDevice] Vulkan instance created.");
		}

		if (CreateSurface() == false)
		{
			return false;
		}
		else
		{
			LOG_BE_INFO("[VulkanDevice] Vulkan surface created.");
		}

		if (SelectPhysicalDevice() == false)
		{
			return false;
		}
		else
		{
			LOG_BE_INFO("[VulkanDevice] Vulkan physical device created.");
		}

		if (CreateLogicalDevice() == false)
		{
			return false;
		}
		else
		{
			LOG_BE_INFO("[VulkanDevice] Vulkan logical device created.");
		}

		if (CreateAllocatorObject() == false)
		{
			return false;
		}
		else
		{
			LOG_BE_INFO("[VulkanDevice] Vulkan allocator object created.");
		}

		if (CreateSwapChain() == false)
		{
			return false;
		}
		else
		{
			LOG_BE_INFO("[VulkanDevice] Vulkan swap chain created.");
		}

		if (CreateCommandsPool() == false)
		{
			return false;
		}
		else
		{
			LOG_BE_INFO("[VulkanDevice] Vulkan commands pool created.");
		}

		if (CreateDepthImage() == false)
		{
			return false;
		}
		else
		{
			LOG_BE_INFO("[VulkanDevice] Vulkan depth image created.");
		}

		if (CreateSwapChainFrameBuffers(true) == false)
		{
			return false;
		}
		else
		{
			LOG_BE_INFO("[VulkanDevice] Vulkan frame buffers created.");
		}


		EventsManager::Get().AddListener(WindowSizeChangedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);

		m_DescriptorLayoutCache = new DescriptorLayoutCache();
		m_DescriptorLayoutCache->Init(m_Device);

		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_DeviceProperties);

		return true;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
		LOG_BE_ERROR("{0} Vulkan Problem: {1}", Tag, pCallbackData->pMessage);
		return VK_FALSE;
	}

	bool VulkanDevice::CreateVulkanInstance()
	{
		if (m_RenderSettings->validationMode == ValidationMode::Enabled && HasValidationLayerSupport() == false)
		{
			LOG_BE_ERROR("{0} Validation mode enabled but not available.", Tag);
			return false;
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "CzuchEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
		appInfo.applicationVersion = VK_API_VERSION_1_3;
		appInfo.apiVersion = VK_API_VERSION_1_3;
		appInfo.pEngineName = "Czuch";

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = GetRequiredExtensions();

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (m_RenderSettings->validationMode == ValidationMode::Enabled)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
			debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugCreateInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugCreateInfo.pfnUserCallback = DebugCallback;
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

		if (glfwCreateWindowSurface(m_Instance, (GLFWwindow*)m_AttachedWindow->GetNativeWindowPtr(), nullptr, &m_Surface) != VK_SUCCESS)
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

		if (m_RenderSettings->validationMode == ValidationMode::Enabled)
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
		}

		if (m_RenderSettings->dynamicRendering)
		{

			VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingCreateInfo{};
			dynamicRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
			dynamicRenderingCreateInfo.dynamicRendering = VK_TRUE;

			VkPhysicalDeviceSynchronization2Features syncFeatures{};
			syncFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
			syncFeatures.synchronization2 = VK_TRUE;

			dynamicRenderingCreateInfo.pNext = &syncFeatures;

			deviceCreateInfo.pNext = &dynamicRenderingCreateInfo;

			//shader draw parameters(gl_BaseInstance)
			VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
			shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
			shader_draw_parameters_features.pNext = nullptr;
			shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;

			dynamicRenderingCreateInfo.pNext = &shader_draw_parameters_features;
		}

		if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
		{
			LOG_BE_ERROR("{0} Failed to Create Vulkan Device.", Tag);
			return false;
		}

		VkPhysicalDeviceDynamicRenderingFeatures checkFeature{};
		checkFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;

		VkPhysicalDeviceFeatures2 checkFeatures2{};
		checkFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		checkFeatures2.pNext = &checkFeature;

		vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &checkFeatures2);

		if (checkFeature.dynamicRendering) {
			LOG_BE_INFO("{0} Dynamic Rendering is supported!", Tag);
		}
		else
		{
			LOG_BE_ERROR("{0} Dynamic Rendering is not supported!", Tag);
		}

		vkGetDeviceQueue(m_Device, families.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, families.presentFamily.value(), 0, &m_PresentQueue);
		return true;
	}

	bool VulkanDevice::CreateDepthImage()
	{
		VkFormat depthStencilFormat = FindDepthFormat(m_PhysicalDevice, true);

		if (depthStencilFormat == VK_FORMAT_UNDEFINED)
		{
			LOG_BE_ERROR("{0} Failed to find fitting depth buffer format", Tag);
			return false;
		}

		auto imageWithAlloc = CreateImage(TextureDesc::Type::TEXTURE_2D, m_SwapChainData.swapChainExtent.width, m_SwapChainData.swapChainExtent.height, 1, ConvertSamplesCount(1), depthStencilFormat,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_SHARING_MODE_EXCLUSIVE);

		if (imageWithAlloc.allocation == nullptr || imageWithAlloc.image == nullptr)
		{
			LOG_BE_ERROR("{0} Failed to create depth buffer", Tag);
			return false;
		}

		m_DepthImage.depthImage = imageWithAlloc.image;
		m_DepthImage.allocation = imageWithAlloc.allocation;

		m_DepthImage.depthImageView = CreateImageView(m_DepthImage.depthImage, depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT,1);
		m_DepthImage.depthFormat = depthStencilFormat;

		TransitionImageLayout(nullptr,m_DepthImage.depthImage, depthStencilFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,0,1,true);
		return true;
	}

	bool VulkanDevice::CreateSwapChain()
	{
		m_SwapChainData = {};
		SwapChainSupportDetails swapchainDetails = QuerySwapChainSupport(m_PhysicalDevice, m_Surface);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainDetails.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainDetails.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapchainDetails.capabilities, (GLFWwindow*)m_AttachedWindow->GetNativeWindowPtr());

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
			m_SwapChainData.swapChainImageViews[i] = CreateImageView(m_SwapChainData.swapChainImages[i], m_SwapChainData.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT,1);
		}

		LOG_BE_INFO("{0} Swap chain created with size {1}x{2}", Tag,m_SwapChainData.swapChainExtent.width,m_SwapChainData.swapChainExtent.height);

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
		if (createRenderPass)
		{
			VulkanRenderPassDesc renderPassDesc;
			renderPassDesc.AddAttachment(ConvertVkFormat(m_SwapChainData.swapChainImageFormat), ImageLayout::PRESENT_SRC_KHR,AttachmentLoadOp::CLEAR);
			renderPassDesc.SetDepthStencilTexture(ConvertVkFormat(m_DepthImage.depthFormat), ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			renderPassDesc.type = RenderPassType::Final;
			renderPassDesc.mainRenderPass = true;
			m_SwapChainRenderPass = CreateRenderPass(&renderPassDesc);
		}

		RenderPass_Vulkan* rpass = Internal_to_RenderPass(AccessRenderPass(m_SwapChainRenderPass));

		for (size_t i = 0; i < m_SwapChainData.swapChainImageViews.size(); i++) {

			FrameBuffer* framebuffer = new FrameBuffer();

			auto& desc = framebuffer->desc;
			desc.isFinalFrameBuffer = true;

			framebuffer->m_InternalResourceState = std::make_shared<FrameBuffer_Vulkan>();
			FrameBufferHandle h;
			h.handle = m_ResContainer.frameBuffers.Add(framebuffer);
			m_SwapChainData.swapChainFrameBuffers[i] = h;

			if (HasDynamicRenderingEnabled())
			{
				continue;
			}

			VkImageView attachments[] = {
				m_SwapChainData.swapChainImageViews[i],
				m_DepthImage.depthImageView,
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = rpass->renderPass;
			framebufferInfo.attachmentCount = 2;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_SwapChainData.swapChainExtent.width;
			framebufferInfo.height = m_SwapChainData.swapChainExtent.height;
			framebufferInfo.layers = 1;

			FrameBuffer_Vulkan* fbuffer = Internal_to_Framebuffer(framebuffer);

			fbuffer->createInfo = framebufferInfo;
			fbuffer->device = m_Device;

			if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &fbuffer->framebuffer) != VK_SUCCESS) {
				LOG_BE_ERROR("{0} Failed to Create Swap chain frame buffers.", Tag);
				return false;
			}
		}

		return true;
	}


	bool VulkanDevice::CreateCommandsPool()
	{
		m_CommandPool = CreateCommandPool(false, true);
		m_CopyCommandPool = CreateCommandPool(true, false);

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
		materialInstances.RemoveAll();
		materials.RemoveAll();
		meshes.RemoveAll();
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

	Mesh* VulkanDevice::AccessMesh(MeshHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			Mesh* result = nullptr;
			m_ResContainer.meshes.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccessMesh with invalid handle.", Tag);
		return nullptr;
	}

	Material* VulkanDevice::AccessMaterial(MaterialHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			Material* result = nullptr;
			m_ResContainer.materials.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccessMaterial with invalid handle.", Tag);
		return nullptr;
	}

	MaterialInstance* VulkanDevice::AccessMaterialInstance(MaterialInstanceHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			MaterialInstance* result = nullptr;
			m_ResContainer.materialInstances.Get(handle.handle, &result);
			return result;
		}
		LOG_BE_ERROR("{0} AccessMaterialInstance with invalid handle.", Tag);
		return nullptr;
	}

	void VulkanDevice::ResizeTexture(TextureHandle texture, U32 width, U32 height)
	{
		Texture* tex = AccessTexture(texture);
		if (tex == nullptr)
		{
			LOG_BE_ERROR("{0} ResizeTexture with invalid handle.", Tag);
			return;
		}

		if (tex->m_InternalResourceState == nullptr)
		{
			LOG_BE_ERROR("{0} ResizeTexture with invalid internal state.", Tag);
			return;
		}

		tex->desc.width = width;
		tex->desc.height = height;
		LOG_BE_INFO("{0} Resizing texture {1} to {2}x{3}", Tag, texture.handle, width, height);
		tex->desc.layoutInfo.currentFormat = ImageLayout::UNDEFINED;
		tex->desc.layoutInfo.lastFormat = ImageLayout::UNDEFINED;
		CreateTexture(&tex->desc,true,texture);
	}

	void VulkanDevice::ResizeFrameBuffer(FrameBufferHandle handle, U32 width, U32 height)
	{
		FrameBuffer* fbuffer = AccessFrameBuffer(handle);
		if (fbuffer == nullptr)
		{
			LOG_BE_ERROR("{0} ResizeFrameBuffer with invalid handle.", Tag);
			return;
		}

		if (fbuffer->m_InternalResourceState == nullptr)
		{
			LOG_BE_ERROR("{0} ResizeFrameBuffer with invalid internal state.", Tag);
			return;
		}

		LOG_BE_INFO("{0} Resizing framebuffer {1} to {2}x{3}", Tag, handle.handle, width, height);
		fbuffer->desc.SetWidthAndHeight(width,height);
		CreateFrameBuffer(&fbuffer->desc, true,handle);	
	}

	void VulkanDevice::TransitionSwapChainImageLayoutPreDraw(VulkanCommandBuffer* cmd, uint32_t imageIndex)
	{
		DoImageMemoryBarrier(
			cmd->GetNativeBuffer(),
			m_SwapChainData.swapChainImages[imageIndex],
			0,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

		bool isDepthStencil = IsDepthFormatWithStencil(ConvertVkFormat(m_DepthImage.depthFormat));
		DoImageMemoryBarrier(
			cmd->GetNativeBuffer(),
			m_DepthImage.depthImage,
			0,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			isDepthStencil? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
	}

	void VulkanDevice::TransitionSwapChainImageLayoutPostDraw(VulkanCommandBuffer* cmd, uint32_t imageIndex)
	{
		DoImageMemoryBarrier(
			cmd->GetNativeBuffer(),
			m_SwapChainData.swapChainImages[imageIndex],
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			0,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
	}

	static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}


	void* VulkanDevice::InitImGUI()
	{
		VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;


		vkCreateDescriptorPool(m_Device, &pool_info, nullptr, &m_ImguiPool);

		IMGUI_CHECKVERSION();
		auto ctx = ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)m_AttachedWindow->GetNativeWindowPtr(), true);
		ImGui_ImplVulkan_InitInfo init_info{};

		init_info.Instance = m_Instance;
		init_info.PhysicalDevice = m_PhysicalDevice;
		init_info.Device = m_Device;
		init_info.Queue = m_GraphicsQueue;
		init_info.DescriptorPool = m_ImguiPool;
		init_info.MinImageCount = 3;
		init_info.ImageCount = 3;
		init_info.UseDynamicRendering = false;
		init_info.CheckVkResultFn = check_vk_result;
		init_info.RenderPass = Internal_to_RenderPass(AccessRenderPass(m_SwapChainRenderPass))->renderPass;
		init_info.UseDynamicRendering = HasDynamicRenderingEnabled();

		init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_SwapChainData.swapChainImageFormat;
		init_info.PipelineRenderingCreateInfo.depthAttachmentFormat = m_DepthImage.depthFormat;
		init_info.PipelineRenderingCreateInfo.stencilAttachmentFormat = m_DepthImage.depthFormat;

		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&init_info);

		ImGui_ImplVulkan_CreateFontsTexture();
		return ctx;
	}

	void VulkanDevice::ShutdownImGUI()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vkDestroyDescriptorPool(m_Device, m_ImguiPool, nullptr);
	}

	void VulkanDevice::TransitionImageLayoutImmediate(TextureHandle handle, ImageLayout oldLayout, ImageLayout newLayout,U32 baseMipLevel, U32 mipCount, bool isDepth)
	{
		auto texture = AccessTexture(handle);
		if (texture == nullptr)
		{
			LOG_BE_ERROR("TransitionImageLayoutImmediate with invalid handle.");
			CZUCH_BE_ASSERT(false,"TransitionImageLayoutImmediate with invalid handle.");
			return;
		}
		VkFormat format = ConvertFormat(texture->desc.format);
		auto textureVulkan = Internal_to_Texture(texture);
		TransitionImageLayout(nullptr,textureVulkan->image, format, ConvertImageLayout(oldLayout), ConvertImageLayout(newLayout),baseMipLevel,mipCount,isDepth);
	}

	void VulkanDevice::TransitionImageLayout(CommandBuffer* cmd, TextureHandle handle, ImageLayout oldLayout, ImageLayout newLayout, U32 baseMipLevel, U32 mipCount, bool isDepth)
	{
		auto texture = AccessTexture(handle);
		if (texture == nullptr)
		{
			LOG_BE_ERROR("TransitionImageLayout with invalid handle.");
			CZUCH_BE_ASSERT(false, "TransitionImageLayout with invalid handle.");
			return;
		}
		VkFormat format = ConvertFormat(texture->desc.format);
		auto textureVulkan = Internal_to_Texture(texture);
		VulkanCommandBuffer* vcmd = (VulkanCommandBuffer*)cmd;
		TransitionImageLayout(vcmd->GetNativeBuffer(), textureVulkan->image, format, ConvertImageLayout(oldLayout), ConvertImageLayout(newLayout), baseMipLevel, mipCount, isDepth);
	}

	bool VulkanDevice::TryTransitionImageLayout(CommandBuffer* cmd, TextureHandle textureHandle, ImageLayout newLayout, U32 baseMipLevel, U32 mipCount)
	{
		auto texture = AccessTexture(textureHandle);
		CZUCH_BE_ASSERT(texture != nullptr, "TryTransitionImageLayout with invalid handle.");

		bool isDepth = IsDepthFormat(texture->desc.format);
		auto targetLayout = newLayout;
		if (texture->desc.layoutInfo.TryToTransitionTo(targetLayout))
		{
			TransitionImageLayout(cmd,textureHandle, texture->desc.layoutInfo.lastFormat, targetLayout, baseMipLevel, mipCount, isDepth);
			return true;
		}
		return false;
	}

	bool VulkanDevice::UploadDataToBuffer(BufferHandle buffer, const void* dataIn, U32 size)
	{
		auto bufferPtr = AccessBuffer(buffer);
		if (bufferPtr == nullptr)
		{
			LOG_BE_ERROR("UploadDataToBuffer with invalid handle.");
			return false;
		}

		auto bufferVulkan = Internal_to_Buffer(bufferPtr);

		BufferInternalSettings settingsStageBuffer{};
		settingsStageBuffer.inSize = bufferVulkan->size;
		settingsStageBuffer.inFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		settingsStageBuffer.inStagingBuffer = true;
		settingsStageBuffer.inCreateMapped = false;
		settingsStageBuffer.inUsage = Usage::MEMORY_USAGE_CPU_ONLY;

		if (!CreateBuffer_Internal(settingsStageBuffer))
		{
			LOG_BE_ERROR("{0} Failed to create new vulkan staging buffer for uniform buffer Upload", Tag);
			return false;
		}

		//map memory
		void* data = nullptr;
		vmaMapMemory(m_VmaAllocator, settingsStageBuffer.outMemAlloc, &data);
		memcpy(data, dataIn, size);
		vmaUnmapMemory(m_VmaAllocator, settingsStageBuffer.outMemAlloc);

		if (!CopyBuffer(settingsStageBuffer.outBuffer, bufferVulkan->buffer, settingsStageBuffer.inSize))
		{
			LOG_BE_ERROR("{0} Failed to copy data from staging buffer to uniform buffer in upload method", Tag);
			vmaDestroyBuffer(m_VmaAllocator, settingsStageBuffer.outBuffer, settingsStageBuffer.outMemAlloc);
			return false;
		}
		vmaDestroyBuffer(m_VmaAllocator, settingsStageBuffer.outBuffer, settingsStageBuffer.outMemAlloc);
		return true;
	}

	bool VulkanDevice::UploadCurrentDataToBuffer(BufferHandle buffer)
	{
		auto bufferPtr = AccessBuffer(buffer);
		if (bufferPtr == nullptr)
		{
			LOG_BE_ERROR("UploadCurrentDataToBuffer with invalid handle.");
			return false;
		}

		void* data = bufferPtr->desc.ubo->GetData();
		return UploadDataToBuffer(buffer, data, bufferPtr->desc.ubo->GetSize());
	}

	Pipeline* VulkanDevice::AccessPipeline(PipelineHandle handle)
	{
		if (HANDLE_IS_VALID(handle))
		{
			Pipeline* result = nullptr;
			bool success = m_ResContainer.pipelines.Get(handle.handle, &result);
			if (success)
			{
				return result;
			}
			LOG_BE_ERROR("{0} AccessPipeline with invalid handle, return from access is equal to false.", Tag);
			return nullptr;
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

	RenderPass* VulkanDevice::GetRenderPassOfType(RenderPassType type)
	{
		auto& renderPasses = m_ResContainer.renderPasses;
		for (auto it=renderPasses.Rbegin_const();it!=renderPasses.Rend_const();it++)
		{
			RenderPass* renderPass =*it;

			if (renderPass == nullptr)
			{
				continue;
			}

			if (renderPass->desc.type == type)
			{
				return *it;
			}
		}
		LOG_BE_ERROR("{0} GetRenderPassHandleOfType with invalid type.", Tag);
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


	void VulkanDevice::DepthImage::Release(VkDevice device, VmaAllocator allocator)
	{
		vkDestroyImageView(device, depthImageView, nullptr);
		vmaDestroyImage(allocator, depthImage, allocation);
	}


}
