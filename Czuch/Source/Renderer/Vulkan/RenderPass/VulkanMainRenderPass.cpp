#include"czpch.h"
#include"VulkanMainRenderPass.h"
#include"Renderer/Vulkan/VulkanRenderer.h"
#include"Renderer/Vulkan/VulkanCommandBuffer.h"
#include"Renderer/Vulkan/DescriptorAllocator.h"
#include"Renderer/Vulkan/VulkanCore.h"
#include"backends/imgui_impl_vulkan.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include"../VulkanDevice.h"


namespace Czuch
{
	VulkanMainRenderPass::VulkanMainRenderPass(VulkanDevice* device, VulkanRenderer* renderer) : VulkanRenderPassControlBase(device,renderer,nullptr,0, 0, RenderPassType::MainForward,true)
	{
		SetPriority(0);
	}

	void VulkanMainRenderPass::PreDraw(CommandBuffer* cmd,Renderer* renderer)
	{

		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;
		int imageIndex = m_Device->GetCurrentImageIndex();

		m_Device->TransitionSwapChainImageLayoutPreDraw(cmdBuffer, imageIndex);

		cmdBuffer->SetClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		cmdBuffer->SetDepthStencil(1.0f, 0);

		if (renderer->GetRenderSettings().dynamicRendering)
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
		vpdesc.width = (U32)m_Device->GetSwapchainWidth();
		vpdesc.height = (U32)m_Device->GetSwapchainHeight();
		cmdBuffer->SetViewport(vpdesc);

		ScissorsDesc scissors{};
		scissors.offsetX = 0;
		scissors.offsetY = 0;
		scissors.width = (U32)m_Device->GetSwapchainWidth();
		scissors.height = (U32)m_Device->GetSwapchainHeight();
		cmdBuffer->SetScrissors(scissors);
	}

	void VulkanMainRenderPass::PostDraw(CommandBuffer* cmd,Renderer* renderer)
	{
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;

		if (renderer->GetRenderSettings().dynamicRendering)
		{
			cmdBuffer->EndDynamicRenderPassForMainPass();
			m_Device->TransitionSwapChainImageLayoutPostDraw(cmdBuffer, m_Device->GetCurrentImageIndex());
		}
		else
		{
			cmdBuffer->EndCurrentRenderPass();
		}
	}


	void VulkanMainRenderPass::Execute(CommandBuffer* cmd)
	{
		VulkanCommandBuffer* cmdBuffer = (VulkanCommandBuffer*)cmd;

		//TODO blit from offscreen to swapchain

		m_Device->DrawUI(cmdBuffer);
	}

	void VulkanMainRenderPass::Resize(int width, int height)
	{
		Release();
	}

	void VulkanMainRenderPass::SetFinalTexture(Texture_Vulkan* texture)
	{
		m_FinalTexture = texture;
	}

}	