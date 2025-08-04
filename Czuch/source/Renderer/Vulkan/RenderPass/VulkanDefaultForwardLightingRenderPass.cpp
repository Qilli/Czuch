#include"czpch.h"
#include "VulkanDefaultForwardLightingRenderPass.h"
#include "../VulkanRenderer.h"
#include"Subsystems/Assets/BuildInAssets.h"
#include "../VulkanDevice.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"


namespace Czuch
{
	VulkanDefaultForwardLightingRenderPass::VulkanDefaultForwardLightingRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize) :VulkanRenderPassControlBase(device,renderer,nullptr, width, height, RenderPassType::ForwardLighting, handleWindowResize)
	{
		m_FillParams.isLightPass = true;
	}

}