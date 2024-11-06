#include"czpch.h"
#include"VulkanOffscreenRenderPass.h"
#include"Renderer/Vulkan/VulkanRenderer.h"
#include"Renderer/Vulkan/DescriptorAllocator.h"
#include"Renderer/Vulkan/VulkanCore.h"
#include"backends/imgui_impl_vulkan.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"

namespace Czuch
{
	VulkanOffscreenRenderPass::VulkanOffscreenRenderPass(Camera* cam, VulkanRenderer* renderer, VulkanDevice* device,
		U32 width, U32 height, bool handleWindowResize) : RenderPassControl(cam, width, height, RenderPassType::OffscreenTexture, handleWindowResize),
		m_Device(device), m_Renderer(renderer)
	{
		Init();
		SetPriority(100);
	}

	VulkanOffscreenRenderPass::~VulkanOffscreenRenderPass()
	{
		Release();
	}

	void VulkanOffscreenRenderPass::PreDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		m_Renderer->OnPreRenderUpdateContexts(nullptr, m_Width, m_Height,nullptr);
	}

	void VulkanOffscreenRenderPass::PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		m_Renderer->OnPostRenderUpdateContexts(nullptr);
	}


	void VulkanOffscreenRenderPass::Execute(CommandBuffer* cmdBuffer)
	{
		m_Renderer->DrawScene((VulkanCommandBuffer*)cmdBuffer);
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
		auto& node=m_FrameGraph->GetNode(m_Node);
		//get first color texture from selected node
		auto colorTexture = node.GetFirstColorAttachment(m_FrameGraph);
		auto texture = m_Device->AccessTexture(colorTexture);
		auto vulkanTexture = Internal_to_Texture(texture);
		m_Descriptor = ImGui_ImplVulkan_AddTexture(vulkanTexture->sampler, vulkanTexture->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanOffscreenRenderPass::Release()
	{

	}

}