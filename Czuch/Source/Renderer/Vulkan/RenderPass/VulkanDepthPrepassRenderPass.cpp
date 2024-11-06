#include"czpch.h"
#include "VulkanDepthPrepassRenderPass.h"
#include "../VulkanRenderer.h"
#include"Subsystems/Assets/BuildInAssets.h"


namespace Czuch
{
	VulkanDepthPrepassRenderPass::VulkanDepthPrepassRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize) :RenderPassControl(nullptr,width,height,RenderPassType::DepthPrePass,handleWindowResize)
	{
		m_Renderer = renderer;
		m_Device = device;
		Init();
	}

	VulkanDepthPrepassRenderPass::~VulkanDepthPrepassRenderPass()
	{
		Release();
	}

	void VulkanDepthPrepassRenderPass::PreDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		renderer->OnPreRenderUpdateContexts(nullptr, m_Width, m_Height,&m_FillParams);
	}

	void VulkanDepthPrepassRenderPass::PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		renderer->OnPostRenderUpdateContexts(&m_FillParams);
	}

	void VulkanDepthPrepassRenderPass::Execute(CommandBuffer* cmdBuffer)
	{
		m_Renderer->DrawScene((VulkanCommandBuffer*)cmdBuffer);
	}

	void VulkanDepthPrepassRenderPass::Resize(int width, int height)
	{
		Release();
		m_Width = width;
		m_Height = height;
		Init();
	}

	void VulkanDepthPrepassRenderPass::Init()
	{
		m_FillParams = { .forcedMaterial = DefaultAssets::DEPTH_PREPASS_MATERIAL_INSTANCE,.forceMaterialForAll=true };
	}

	void VulkanDepthPrepassRenderPass::Release()
	{

	}

}