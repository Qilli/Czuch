#include"czpch.h"
#include"VulkanOffscreenRenderPass.h"
#include"Renderer/Vulkan/VulkanRenderer.h"
#include"Renderer/Vulkan/DescriptorAllocator.h"
#include"Renderer/Vulkan/VulkanCore.h"
#include"backends/imgui_impl_vulkan.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"

namespace Czuch
{
	VulkanOffscreenRenderPass::VulkanOffscreenRenderPass(Camera* cam,VulkanRenderer* renderer, VulkanDevice* device, Format colorFormat, Format depthFormat, U32 width, U32 height, VkSampleCountFlagBits samples,bool handleWindowResize) : RenderPassControl(cam,width, height, RenderPassType::OffscreenTexture,handleWindowResize),
		m_Device(device), m_ColorFormat(colorFormat), m_DepthFormat(depthFormat), m_Renderer(renderer)
	{
		Init();
		SetPriority(100);
	}

	VulkanOffscreenRenderPass::~VulkanOffscreenRenderPass()
	{
		Release();
	}

	void VulkanOffscreenRenderPass::BeginRenderPass(CommandBuffer* cmdBuffer)
	{
		
	}

	void VulkanOffscreenRenderPass::EndRenderPass(CommandBuffer* cmdBuffer)
	{
	}

	void VulkanOffscreenRenderPass::DrawSceneToOffscreenBuffer(VulkanCommandBuffer* cmdBuffer)
	{
		m_Device->BeginOffscreenPass(cmdBuffer,m_RenderPass,m_FrameBuffer,m_Width,m_Height);
		m_Renderer->DrawScene(cmdBuffer);
		m_Device->EndOffscreenPass(cmdBuffer);
	}

	void VulkanOffscreenRenderPass::Execute(CommandBuffer* cmdBuffer)
	{
		m_Device->TransitionImageLayout(m_Color,ImageLayout::UNDEFINED, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		m_Renderer->ImmediateSubmitWithCommandBuffer([this](CommandBuffer* cmd) {
			DrawSceneToOffscreenBuffer((VulkanCommandBuffer*)cmd);
			m_Device->TransitionImageLayout(m_Color, ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			});
	}

	void VulkanOffscreenRenderPass::Resize(int width, int height)
	{
		Release();
		m_Width = width;
		m_Height = height;
		Init();
	}

	void* VulkanOffscreenRenderPass::GetRenderPassResult()
	{
		return (void*)m_Descriptor;
	}

	void VulkanOffscreenRenderPass::Init()
	{
		//color
		SamplerDesc samplerDesc{};
		samplerDesc.magFilter = TextureFilter::LINEAR;
		samplerDesc.minFilter = TextureFilter::LINEAR;
		samplerDesc.addressModeU = TextureAddressMode::EDGE_CLAMP;
		samplerDesc.addressModeV = TextureAddressMode::EDGE_CLAMP;
		samplerDesc.addressModeW = TextureAddressMode::EDGE_CLAMP;

		TextureDesc texDesc{};
		texDesc.type = TextureDesc::Type::TEXTURE_2D;
		texDesc.width = GetWidth();
		texDesc.height = GetHeight();
		texDesc.depth = 1;
		texDesc.format = m_ColorFormat;
		texDesc.samplerDesc = samplerDesc;
		texDesc.sample_count = 1;
		texDesc.initialLayout = ImageLayout::UNDEFINED;
		texDesc.usageFlags = ImageUsageFlag::COLOR_ATTACHMENT | ImageUsageFlag::SAMPLED;
		texDesc.aspectFlags = ImageAspectFlag::COLOR;
		texDesc.resourceType = ResourceState::SHADER_RESOURCE;


		m_Color=m_Device->CreateTexture(&texDesc);

		//depth
		TextureDesc depthTexDesc{};
		depthTexDesc.type = TextureDesc::Type::TEXTURE_2D;
		depthTexDesc.width = GetWidth();
		depthTexDesc.height = GetHeight();
		depthTexDesc.depth = 1;
		depthTexDesc.format = m_DepthFormat;
		depthTexDesc.samplerDesc = samplerDesc;
		depthTexDesc.sample_count = 1;
		depthTexDesc.initialLayout = ImageLayout::UNDEFINED;
		depthTexDesc.usageFlags = ImageUsageFlag::DEPTH_STENCIL_ATTACHMENT;
		depthTexDesc.aspectFlags = ImageAspectFlag::DEPTH|ImageAspectFlag::STENCIL;
		depthTexDesc.resourceType = ResourceState::DEPTHSTENCIL;
		m_Depth= m_Device->CreateTexture(&depthTexDesc);

		//render pass
		std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
		// Color attachment
		attchmentDescriptions[0].format = ConvertFormat(m_ColorFormat);
		attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		// Depth attachment
		attchmentDescriptions[1].format = ConvertFormat(m_DepthFormat);
		attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_NONE_KHR;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
		renderPassInfo.pAttachments = attchmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VulkanRenderPassDesc desc;
		desc.renderPassInfo = renderPassInfo;
		desc.mainRenderPass = false;
		auto renderPassHandle =m_Device->CreateRenderPass(&desc);


		if (!HANDLE_IS_VALID(renderPassHandle))
		{
			LOG_BE_ERROR("{0} Failed to create offscreen render pass.", "VulkanOffscreenRenderPass");
			return;
		}

		m_ColorTexture= Internal_to_Texture(m_Device->AccessTexture(m_Color));
		m_DepthTexture = Internal_to_Texture(m_Device->AccessTexture(m_Depth));

		VulkanFrameBufferDesc fbDesc{};
		fbDesc.color = m_ColorTexture->imageView;
		fbDesc.depth = m_DepthTexture->imageView;
		fbDesc.width = GetWidth();
		fbDesc.height = GetHeight();
		fbDesc.renderPass = renderPassHandle;

		FrameBufferHandle handle = m_Device->CreateFrameBuffer(&fbDesc);
		m_FrameBuffer = handle;
		m_Descriptor = ImGui_ImplVulkan_AddTexture(m_ColorTexture->sampler, m_ColorTexture->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_RenderPass = Internal_to_RenderPass(m_Device->AccessRenderPass(renderPassHandle))->renderPass;
		m_RenderPassHandle = renderPassHandle;
	}

	void VulkanOffscreenRenderPass::Release()
	{
		m_Device->Release(m_FrameBuffer);
		m_Device->Release(m_RenderPassHandle);
		m_Device->Release(m_Color);
		m_Device->Release(m_Depth);	
	}

}