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

	void VulkanDebugDrawRenderPass::Init()
	{
		VulkanRenderPassControlBase::Init();
		m_FillParams.renderPassType = RenderPassType::DebugDraw;
		m_FillParams.forceMaterialForAll = true;
		m_FillParams.forcedMaterial = DefaultAssets::DEBUG_DRAW_LIGHT_MATERIAL_INSTANCE;

		m_DebugFillParams[0].renderPassType = RenderPassType::DebugDraw;
		m_DebugFillParams[0].forceMaterialForAll = true;
		m_DebugFillParams[0].forcedMaterial = DefaultAssets::DEBUG_DRAW_LINES_MATERIAL_INSTANCE;

		m_DebugFillParams[1].renderPassType = RenderPassType::DebugDraw;
		m_DebugFillParams[1].forceMaterialForAll = true;
		m_DebugFillParams[1].forcedMaterial = DefaultAssets::DEBUG_DRAW_TRIANGLES_MATERIAL_INSTANCE;

		m_DebugFillParams[2].renderPassType = RenderPassType::DebugDraw;
		m_DebugFillParams[2].forceMaterialForAll = true;
		m_DebugFillParams[2].forcedMaterial = DefaultAssets::DEBUG_DRAW_POINTS_MATERIAL_INSTANCE;
	}

	void VulkanDebugDrawRenderPass::Execute(CommandBuffer* cmdBuffer)
	{
		//render debug draw lines/triangles/points
		//VulkanRenderPassControlBase::Execute(cmdBuffer);

		m_FillParams.forcedMaterial = DefaultAssets::DEBUG_DRAW_LINES_MATERIAL_INSTANCE;
		m_Renderer->DrawDebugData((VulkanCommandBuffer*)cmdBuffer, m_Camera, m_DebugFillParams);
	}

	void VulkanDebugDrawRenderPass::BeforeFrameGraphExecute(CommandBuffer* cmd, Renderer* renderer)
	{
		VulkanRenderPassControlBase::BeforeFrameGraphExecute(cmd, renderer);
		//gather debug draw data
		renderer->OnPreRenderUpdateDebugDrawElements(m_Camera, &m_FillParams);
	}

}