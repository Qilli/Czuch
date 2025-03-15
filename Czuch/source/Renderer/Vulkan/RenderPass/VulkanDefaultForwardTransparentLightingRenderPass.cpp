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

	void VulkanDefaultForwardTransparentLightingRenderPass::CreateRenderContexts()
	{
		RenderContextCreateInfo renderContextCreateInfo{};
		renderContextCreateInfo.autoClearAfterRender = true;
		renderContextCreateInfo.renderType = RenderType::General;
		renderContextCreateInfo.renderLayer = RenderLayer::LAYER_0;
		renderContextCreateInfo.sortingOrder = 0;
		m_GeneralRenderContext = new DefaultTransparentRenderContext(renderContextCreateInfo);

		renderContextCreateInfo.renderType = RenderType::Debug;
		renderContextCreateInfo.renderLayer = RenderLayer::LAYER_0;
		m_DebugRenderContext = new DefaultTransparentRenderContext(renderContextCreateInfo);
	}

}