#include "czpch.h"
#include "VulkanRenderer.h"
#include"VulkanDevice.h"
#include"VulkanCommandBuffer.h"
#include"VulkanCore.h"
#include"Platform/Windows/WinWindow.h"

#include"Subsystems/Assets/AssetsManager.h"
#include"Subsystems/Assets/Asset/ShaderAsset.h"
#include"Subsystems/Assets/Asset/TextureAsset.h"
#include"Subsystems/Assets/Asset/MaterialAsset.h"
#include"Subsystems/Scenes/Scene.h"
#include"Renderer/RenderContext.h"
#include"DescriptorAllocator.h"
#include"Core/Math.h"

namespace Czuch
{
	VulkanRenderer::VulkanRenderer(Window* window,ValidationMode validationMode)
	{
		m_RendererValidationMode = validationMode;
		m_AttachedWindow = window;
		m_ActiveScene = nullptr;
	}


	BufferHandle vertexBufferPos = INVALID_HANDLE(BufferHandle);
	BufferHandle vertexBufferColor = INVALID_HANDLE(BufferHandle);
	BufferHandle vertexBufferUV = INVALID_HANDLE(BufferHandle);
	ShaderHandle vertexShader = INVALID_HANDLE(ShaderHandle);
	BufferHandle indexBuffer = INVALID_HANDLE(BufferHandle);
	ShaderHandle fragmentShader = INVALID_HANDLE(ShaderHandle);
	bool inited = false;

	VulkanRenderer::~VulkanRenderer()
	{
		m_Device->AwaitDevice();


		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_FramesData[a].frameDeletionQueue.Flush();
		}
		
		ReleaseSyncObjects();
		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_FramesData[a].descriptorAllocator->CleanUp();
			m_Device->ReleaseDescriptorAllocator(m_FramesData[a].descriptorAllocator);
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

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_FramesData[a].commandBuffer = m_Device->CreateCommandBuffer(true);
			VulkanCommandBuffer* cmd = (VulkanCommandBuffer*)m_Device->AccessCommandBuffer(m_FramesData[a].commandBuffer);
			cmd->Init(m_Device);

			m_FramesData[a].descriptorAllocator = m_Device->CreateDescriptorAllocator();
		}

		CreateSyncObjects();
		InitSceneData();
	}

	void VulkanRenderer::DrawFrame()
	{
		VkDevice device = m_Device->GetNativeDevice();
		vkWaitForFences(device, 1, &GetCurrentFrame().inFlightFence, VK_TRUE, UINT64_MAX);
		GetCurrentFrame().Reset();
		GetCurrentFrame().frameDeletionQueue.Flush();

		bool failedToAcquire = false;
		uint32_t imageIndex = m_Device->AcquireNextSwapChainImage(GetCurrentFrame().imageAvailableSemaphore,failedToAcquire);
		if (failedToAcquire)
		{
			return;
		}

		SetSceneData();
		OnPreRenderUpdateContexts();
		vkResetFences(device, 1, &GetCurrentFrame().inFlightFence);
		
		RecordCommandBuffer(imageIndex);

		SubmitCommandBuffer();
		m_Device->Present(imageIndex, GetCurrentFrame().renderFinishedSemaphote);

		OnPostRenderUpdateContexts();
		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::AwaitDeviceIdle()
	{
		m_Device->AwaitDevice();
	}

	GraphicsDevice* VulkanRenderer::GetDevice()
	{
		 return static_cast<GraphicsDevice*>(m_Device); 
	}

	bool VulkanRenderer::RegisterRenderContext(RenderContext* context)
	{
		m_MainRenderContexts.Add(context);
		return true;
	}

	void VulkanRenderer::UnRegisterRenderContext(RenderContext* context)
	{
		m_MainRenderContexts.Remove(context);
	}

	void VulkanRenderer::SetActiveScene(Scene* scene)
	{
		if (m_ActiveScene == scene)
		{
			return;
		}

		if (m_ActiveScene != nullptr)
		{
			for (auto context : m_MainRenderContexts.m_RenderContexts)
			{
				context->ClearRenderList();
			}

			UnRegisterRenderContext(&m_ActiveScene->GetGeneralRenderContext());
			UnRegisterRenderContext(&m_ActiveScene->GetUIRenderContext());
			UnRegisterRenderContext(&m_ActiveScene->GetDebugRenderContext());
		}

		m_ActiveScene = scene;

		if (m_ActiveScene != nullptr)
		{
			RegisterRenderContext(&m_ActiveScene->GetGeneralRenderContext());
			RegisterRenderContext(&m_ActiveScene->GetUIRenderContext());
			RegisterRenderContext(&m_ActiveScene->GetDebugRenderContext());
		}
	}

	void VulkanRenderer::CreateSyncObjects()
	{

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_FramesData[a].imageAvailableSemaphore = m_Device->CreateNewSemaphore();
			m_FramesData[a].renderFinishedSemaphote = m_Device->CreateNewSemaphore();
			m_FramesData[a].inFlightFence = m_Device->CreateNewFence(true);
		}
	}

	void VulkanRenderer::ReleaseSyncObjects()
	{
		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_Device->ReleaseSemaphore(m_FramesData[a].imageAvailableSemaphore);
			m_Device->ReleaseSemaphore(m_FramesData[a].renderFinishedSemaphote);
			m_Device->ReleaseFence(m_FramesData[a].inFlightFence);
		}
	}

	void VulkanRenderer::RecordCommandBuffer(uint32_t imageIndex)
	{
		auto cmdBuffer = m_Device->AccessCommandBuffer(GetCurrentFrame().commandBuffer);
		cmdBuffer->Begin();

		cmdBuffer->SetClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		cmdBuffer->SetDepthStencil(1.0f, 0);
		m_Device->BindSwapChainRenderPass(cmdBuffer, imageIndex);

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

		for (auto context : m_MainRenderContexts.m_RenderContexts)
		{
			for (auto &renderItem : context->GetRenderObjectsList())
			{
				cmdBuffer->DrawMesh(renderItem, GetCurrentFrame().descriptorAllocator);
			}
		}

		cmdBuffer->EndCurrentRenderPass();
		cmdBuffer->End();
	}

	void VulkanRenderer::SubmitCommandBuffer()
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkCommandBuffer cmdBuffer = ((VulkanCommandBuffer*)m_Device->AccessCommandBuffer(GetCurrentFrame().commandBuffer))->GetNativeBuffer();

		VkSemaphore waitSemaphores[] = { GetCurrentFrame().imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VkSemaphore signalSemaphores[] = { GetCurrentFrame().renderFinishedSemaphote };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		m_Device->SubmitToGraphicsQueue(submitInfo, GetCurrentFrame().inFlightFence);
	}

	void VulkanRenderer::InitSceneData()
	{

		m_SceneData.bufferDesc.createMapped = true;
		m_SceneData.bufferDesc.elementsCount = 1;
		m_SceneData.bufferDesc.bind_flags = BindFlag::UNIFORM_BUFFER;
		m_SceneData.bufferDesc.size = sizeof(SceneData);
		m_SceneData.bufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		m_SceneData.data.ambientColor = Vec4(1, 0, 0, 1);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			INVALIDATE_HANDLE(m_SceneData.buffer[a]);
		}

	}

	void VulkanRenderer::SetSceneData()
	{
		m_SceneData.buffer[m_CurrentFrame] = m_Device->CreateBuffer(&m_SceneData.bufferDesc);
		GetCurrentFrame().frameDeletionQueue.PushFunction([=, this]() {
			if (HANDLE_IS_VALID(m_SceneData.buffer[m_CurrentFrame])) 
			{ 
				m_Device->Release(m_SceneData.buffer[m_CurrentFrame]); } 
			});
		auto bufferVulkan =Internal_to_Buffer(m_Device->AccessBuffer(m_SceneData.buffer[m_CurrentFrame]));

		m_SceneData.data.ambientColor = Vec4(1, 1, 0, 1);

		SceneData* data=(SceneData*)bufferVulkan->GetMappedData();
		*data = m_SceneData.data;

	}

	void VulkanRenderer::OnPreRenderUpdateContexts()
	{
		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->FillRenderContexts(this);
		}

		for (auto context: m_MainRenderContexts.m_RenderContexts)
		{
			auto renderList=context->GetRenderObjectsList();
			for (auto renderElement : renderList)
			{
				renderElement.UpdateSceneDataIfRequired(m_Device,m_SceneData.buffer[m_CurrentFrame]);
			}
		}
	}

	void VulkanRenderer::OnPostRenderUpdateContexts()
	{
		for (auto context : m_MainRenderContexts.m_RenderContexts)
		{
			if (context->IsAutoCleanEnabled())
			{
				context->ClearRenderList();
			}
		}
	}

	void VulkanRenderer::FrameData::Reset()
	{
		descriptorAllocator->ResetPools();
	}

}
