#include"czpch.h"
#include "VulkanDefaultForwardLightingRenderPass.h"
#include "../VulkanRenderer.h"
#include"Subsystems/Assets/BuildInAssets.h"


namespace Czuch
{
	VulkanDefaultForwardLightingRenderPass::VulkanDefaultForwardLightingRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize) :RenderPassControl(nullptr, width, height, RenderPassType::DepthPrePass, handleWindowResize)
	{
		m_Renderer = renderer;
		m_Device = device;
		Init();
	}

	VulkanDefaultForwardLightingRenderPass::~VulkanDefaultForwardLightingRenderPass()
	{
		Release();
	}

	void VulkanDefaultForwardLightingRenderPass::PreDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		renderer->OnPreRenderUpdateContexts(nullptr, m_Width, m_Height, &m_FillParams);
	}

	void VulkanDefaultForwardLightingRenderPass::PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		renderer->OnPostRenderUpdateContexts(&m_FillParams);
	}

	void VulkanDefaultForwardLightingRenderPass::Execute(CommandBuffer* cmdBuffer)
	{
		m_Renderer->DrawScene((VulkanCommandBuffer*)cmdBuffer);
	}

	void VulkanDefaultForwardLightingRenderPass::Resize(int width, int height)
	{
		Release();
		m_Width = width;
		m_Height = height;
		Init();
	}

	void VulkanDefaultForwardLightingRenderPass::Init()
	{
		m_FillParams = {};
	}

	void VulkanDefaultForwardLightingRenderPass::Release()
	{

	}

}