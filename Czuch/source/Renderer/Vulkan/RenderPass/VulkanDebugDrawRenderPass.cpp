#include"czpch.h"
#include "VulkanDebugDrawRenderPass.h"
#include "../VulkanRenderer.h"
#include"Subsystems/Assets/BuildInAssets.h"
#include "../VulkanDevice.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"


namespace Czuch
{
	VulkanDebugDrawRenderPass::VulkanDebugDrawRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize) :VulkanRenderPassControlBase(device,renderer,nullptr, width, height, RenderPassType::DebugDraw, handleWindowResize)
	{

	}

	void VulkanDebugDrawRenderPass::CreateRenderContexts()
	{
		RenderContextCreateInfo renderContextCreateInfo{};
		renderContextCreateInfo.autoClearAfterRender = true;
		renderContextCreateInfo.renderType = RenderType::Debug;
		renderContextCreateInfo.renderLayer = RenderLayer::LAYER_0;
		renderContextCreateInfo.sortingOrder = 0;
		m_GeneralRenderContext = new DebugRenderContext(renderContextCreateInfo);

		renderContextCreateInfo.renderType = RenderType::Debug;
		renderContextCreateInfo.renderLayer = RenderLayer::LAYER_0;
		m_DebugRenderContext = new DebugRenderContext(renderContextCreateInfo);
	}

}