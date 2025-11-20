#include"czpch.h"
#include"VulkanDirectionalShadowMapRenderPass.h"
#include"Subsystems/Assets/BuildInAssets.h"

namespace Czuch
{
	VulkanDirectionalShadowMapRenderPass::VulkanDirectionalShadowMapRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize) :VulkanRenderPassControlBase(device, renderer, nullptr, width, height, RenderPassType::DirectionalShadowMap, handleWindowResize)
	{
	}


	void VulkanDirectionalShadowMapRenderPass::Init()
	{
		VulkanRenderPassControlBase::Init();
		m_FillParams.forcedMaterial = DefaultAssets::DIRECTIONAL_SHADOWMAP_MATERIAL_INSTANCE;
		m_FillParams.forceMaterialForAll = true;
		m_FillParams.ignoreTransparent = true;
		m_FillParams.isDirectionalShadowPass = true;
	}

}