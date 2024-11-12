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
		m_FillParams = { .forcedMaterial = DefaultAssets::DEPTH_PREPASS_MATERIAL_INSTANCE,.forceMaterialForAll = true };
	}
}