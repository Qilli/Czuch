#include"czpch.h"
#include "VulkanDepthPrepassRenderPass.h"
#include "../VulkanRenderer.h"
#include"Subsystems/Assets/BuildInAssets.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include "../VulkanDevice.h"


namespace Czuch
{
	VulkanDepthPrepassRenderPass::VulkanDepthPrepassRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize) :VulkanRenderPassControlBase(device,renderer,nullptr,width,height,RenderPassType::DepthPrePass,handleWindowResize)
	{
		
	}
	void* VulkanDepthPrepassRenderPass::GetRenderPassResult()
	{
		return nullptr;
	}
	void VulkanDepthPrepassRenderPass::Init()
	{
		VulkanRenderPassControlBase::Init();
		m_FillParams.forcedMaterial = DefaultAssets::DEPTH_PREPASS_MATERIAL_INSTANCE;
		m_FillParams.forceMaterialForAll = true;
		m_FillParams.ignoreTransparent = true;
	}
}