#include"czpch.h"
#include"VulkanMainRenderPass.h"
#include"Renderer/Vulkan/VulkanRenderer.h"
#include"Renderer/Vulkan/VulkanCommandBuffer.h"
#include"Renderer/Vulkan/DescriptorAllocator.h"
#include"Renderer/Vulkan/VulkanCore.h"
#include"backends/imgui_impl_vulkan.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"


namespace Czuch
{
	VulkanMainRenderPass::VulkanMainRenderPass(RenderSettings* settings,VulkanRenderer* renderer, VulkanDevice* device) : RenderPassControl(settings,nullptr,0, 0, RenderPassType::MainForward,false),
		m_Device(device), m_Renderer(renderer)
	{
		Init();
		SetPriority(0);
	}

	VulkanMainRenderPass::~VulkanMainRenderPass()
	{
		Release();
	}

	void VulkanMainRenderPass::BeginRenderPass(CommandBuffer* cmd)
	{
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;
		int imageIndex = m_Device->GetCurrentImageIndex();
		cmdBuffer->Begin();

		m_Device->TransitionSwapChainImageLayoutPreDraw(cmdBuffer, imageIndex);

		cmdBuffer->SetClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		cmdBuffer->SetDepthStencil(1.0f, 0);

		if (m_Renderer->GetRenderSettings().dynamicRendering)
		{
			m_Device->StartDynamicRenderPass(cmdBuffer, imageIndex);
		}
		else
		{
			m_Device->BindSwapChainRenderPass(cmdBuffer, imageIndex);
		}

		ViewportDesc vpdesc{};
		vpdesc.x = 0;
		vpdesc.y = 0;
		vpdesc.minDepth = 0.0f;
		vpdesc.maxDepth = 1.0f;
		vpdesc.width = m_Device->GetSwapchainWidth();
		vpdesc.height = m_Device->GetSwapchainHeight();
		cmdBuffer->SetViewport(vpdesc);

		ScissorsDesc scissors{};
		scissors.offsetX = 0;
		scissors.offsetY = 0;
		scissors.width = m_Device->GetSwapchainWidth();
		scissors.height = m_Device->GetSwapchainHeight();
		cmdBuffer->SetScrissors(scissors);
	}

	void VulkanMainRenderPass::EndRenderPass(CommandBuffer* cmd)
	{
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;

		if (m_Renderer->GetRenderSettings().dynamicRendering)
		{
			cmdBuffer->EndDynamicRenderPass();
			m_Device->TransitionSwapChainImageLayoutPostDraw(cmdBuffer, m_Device->GetCurrentImageIndex());
		}
		else
		{
			cmdBuffer->EndCurrentRenderPass();
		}
		cmdBuffer->End();
	}


	void VulkanMainRenderPass::Execute(CommandBuffer* cmd)
	{
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;
		if (m_Renderer->HasRenderPass(RenderPassType::OffscreenTexture)==false)
		{
			m_Renderer->DrawScene(cmdBuffer);
		}

		m_Device->DrawUI(cmdBuffer);
	}

	void VulkanMainRenderPass::Resize(int width, int height)
	{
		Release();
		Init();
	}

	void VulkanMainRenderPass::Init()
	{

	}

	void VulkanMainRenderPass::Release()
	{
	}
}	