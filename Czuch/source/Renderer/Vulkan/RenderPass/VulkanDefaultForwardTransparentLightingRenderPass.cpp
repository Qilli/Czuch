#include"czpch.h"
#include "VulkanDefaultForwardTransparentLightingRenderPass.h"
#include "../VulkanRenderer.h"
#include"Subsystems/Assets/BuildInAssets.h"
#include "../VulkanDevice.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"


namespace Czuch
{
	VulkanDefaultForwardTransparentLightingRenderPass::VulkanDefaultForwardTransparentLightingRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize) :VulkanRenderPassControlBase(device,renderer,nullptr, width, height, RenderPassType::ForwardLightingTransparent, handleWindowResize)
	{

	}
}