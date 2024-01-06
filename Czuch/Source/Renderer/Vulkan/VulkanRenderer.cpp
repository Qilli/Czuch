#include "czpch.h"
#include "VulkanRenderer.h"
#include"VulkanDevice.h"
#include"VulkanCommandBuffer.h"
#include"VulkanCore.h"
#include"Platform/Windows/WinWindow.h"

namespace Czuch
{
	VulkanRenderer::VulkanRenderer(Window* window,ValidationMode validationMode)
	{
		m_RendererValidationMode = validationMode;
		m_AttachedWindow = window;
	}


	VulkanRenderer::~VulkanRenderer()
	{
		ReleaseSyncObjects();
		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_Device->ReleaseCommandBuffer(m_CmdBuffers[a]);
		}
		
		delete m_Device;
	}

	void VulkanRenderer::Init()
	{
		m_Device = new VulkanDevice(m_AttachedWindow, m_RendererValidationMode);
		bool init=m_Device->InitDevice();

		if (init == false)
		{
			LOG_BE_ERROR("[Vulkan]Failed to create vulkan device.");
		}

		m_CmdBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_CmdBuffers[a] = (VulkanCommandBuffer*)m_Device->CreateCommandBuffer(true);
			m_CmdBuffers[a]->Init(m_Device);
		}

		CreateSyncObjects();
	}

	void VulkanRenderer::DrawFrame()
	{
		VkDevice device = m_Device->GetNativeDevice();
		vkWaitForFences(device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		
		bool failedToAcquire = false;
		uint32_t imageIndex = m_Device->AcquireNextSwapChainImage(m_ImageAvailableSemaphores[m_CurrentFrame],failedToAcquire);
		if (failedToAcquire)
		{
			return;
		}
		vkResetFences(device, 1, &m_InFlightFences[m_CurrentFrame]);
		
		RecordCommandBuffer(imageIndex);

		SubmitCommandBuffer();
		m_Device->Present(imageIndex, m_RenderFinishedSemaphores[m_CurrentFrame]);

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::AwaitDeviceIdle()
	{
		vkDeviceWaitIdle(m_Device->GetNativeDevice());
	}

	GraphicsDevice* VulkanRenderer::GetDevice()
	{
		 return static_cast<GraphicsDevice*>(m_Device); 
	}

	void VulkanRenderer::CreateSyncObjects()
	{
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_ImageAvailableSemaphores[a] = m_Device->CreateNewSemaphore();
			m_RenderFinishedSemaphores[a] = m_Device->CreateNewSemaphore();
			m_InFlightFences[a] = m_Device->CreateNewFence(true);
		}
	}

	void VulkanRenderer::ReleaseSyncObjects()
	{
		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_Device->ReleaseSemaphore(m_ImageAvailableSemaphores[a]);
			m_Device->ReleaseSemaphore(m_RenderFinishedSemaphores[a]);
			m_Device->ReleaseFence(m_InFlightFences[a]);
		}
	}

	void VulkanRenderer::RecordCommandBuffer(uint32_t imageIndex)
	{
		m_CmdBuffers[m_CurrentFrame]->Begin();
		m_Device->BindSwapChainRenderPass(m_CmdBuffers[m_CurrentFrame], imageIndex);

		ViewportDesc vpdesc{};
		vpdesc.x = 0;
		vpdesc.y = 0;
		vpdesc.minDepth = 0.0f;
		vpdesc.maxDepth = 1.0f;
		vpdesc.width = m_Device->GetSwapchainWidth();
		vpdesc.height = m_Device->GetSwapchainHeight();
		m_CmdBuffers[m_CurrentFrame]->SetViewport(vpdesc);

		ScissorsDesc scissors{};
		scissors.offsetX = 0;
		scissors.offsetY = 0;
		scissors.width = m_Device->GetSwapchainWidth();
		scissors.height = m_Device->GetSwapchainHeight();
		m_CmdBuffers[m_CurrentFrame]->SetScrissors(scissors);

		m_CmdBuffers[m_CurrentFrame]->EndCurrentRenderPass();
		m_CmdBuffers[m_CurrentFrame]->End();
	}

	void VulkanRenderer::SubmitCommandBuffer()
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkCommandBuffer cmdBuffer = m_CmdBuffers[m_CurrentFrame]->GetNativeBuffer();

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		m_Device->SubmitToGraphicsQueue(submitInfo, m_InFlightFences[m_CurrentFrame]);
	}

}
