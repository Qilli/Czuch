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

#include"RenderPass/VulkanMainRenderPass.h"
#include"RenderPass/VulkanOffscreenRenderPass.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"

namespace Czuch
{
	VulkanRenderer::VulkanRenderer(Window* window, RenderSettings* renderSettings)
	{
		m_AttachedWindow = window;
		m_ActiveScene = nullptr;
		m_RenderSettings = renderSettings;
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

		for (auto renderPass : m_RenderPasses)
		{
			delete renderPass;
		}

		m_ImmediateSubmitData.Release(m_Device);

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
		m_Device = new VulkanDevice(m_AttachedWindow);

		bool init = m_Device->InitDevice(m_RenderSettings);

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
		InitImmediateSubmitData();

		if (m_RenderSettings->mainRenderPassActive)
		{
			AddRenderPass(new VulkanMainRenderPass(this, m_Device));
		}

	}

	void VulkanRenderer::DrawFrame()
	{
		VkDevice device = m_Device->GetNativeDevice();
		vkWaitForFences(device, 1, &GetCurrentFrame().inFlightFence, VK_TRUE, UINT64_MAX);
		GetCurrentFrame().Reset();
		GetCurrentFrame().frameDeletionQueue.Flush();

		m_Device->PreDrawFrame();

		bool failedToAcquire = false;
		uint32_t imageIndex = m_Device->AcquireNextSwapChainImage(GetCurrentFrame().imageAvailableSemaphore, failedToAcquire);
		if (failedToAcquire)
		{
			return;
		}

		SetSceneData();
		OnPreRenderUpdateContexts(nullptr, m_Device->GetSwapchainWidth(), m_Device->GetSwapchainHeight());
		vkResetFences(device, 1, &GetCurrentFrame().inFlightFence);

		int lastWidth = m_Device->GetSwapchainWidth();
		int lastHeight = m_Device->GetSwapchainHeight();
		Camera* currentCamera = nullptr;


		for (auto it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it)
		{
			if ((*it)->GetType() == RenderPassType::MainForward)
			{
				(*it)->BeginRenderPass(m_Device->AccessCommandBuffer(GetCurrentFrame().commandBuffer));
				(*it)->Execute(m_Device->AccessCommandBuffer(GetCurrentFrame().commandBuffer));
				(*it)->EndRenderPass(m_Device->AccessCommandBuffer(GetCurrentFrame().commandBuffer));
			}
			else
			{
				if ((*it)->IsDifferentAspect(lastWidth, lastHeight) || (*it)->IsDifferentCamera(currentCamera))
				{
					OnPostRenderUpdateContexts();
					lastWidth = (*it)->GetWidth();
					lastHeight = (*it)->GetHeight();
					currentCamera = (*it)->GetCamera();
					OnPreRenderUpdateContexts(currentCamera, lastWidth, lastHeight);
				}
				(*it)->Execute(m_Device->AccessCommandBuffer(GetCurrentFrame().commandBuffer));
				(*it)->BeginRenderPass(nullptr);
				(*it)->Execute(nullptr);
				(*it)->EndRenderPass(nullptr);
			}
		}


		SubmitCommandBuffer();
		m_Device->Present(imageIndex, GetCurrentFrame().renderFinishedSemaphote);

		CheckForResizeQueries();

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
			UnRegisterRenderContext(&m_ActiveScene->GetDebugRenderContext());
		}

		m_ActiveScene = scene;

		if (m_ActiveScene != nullptr)
		{
			RegisterRenderContext(&m_ActiveScene->GetGeneralRenderContext());
			RegisterRenderContext(&m_ActiveScene->GetDebugRenderContext());
		}
	}

	void VulkanRenderer::ImmediateSubmitWithCommandBuffer(std::function<void(CommandBuffer* cmd)>&& processor)
	{
		auto device = m_Device->GetNativeDevice();
		vkResetFences(device, 1, &m_ImmediateSubmitData.fence);
		VulkanCommandBuffer* cmdBuffer = static_cast<VulkanCommandBuffer*>(m_Device->AccessCommandBuffer(m_ImmediateSubmitData.commandBuffer));
		cmdBuffer->Begin(CommandBufferUseFlag::ONE_TIME_SUBMIT);

		processor(cmdBuffer);

		cmdBuffer->End();

		m_Device->ImmediateSubmitToGraphicsQueueWithCommandBuffer(cmdBuffer->GetNativeBuffer(), m_ImmediateSubmitData.fence);
	}

	void VulkanRenderer::DrawScene(VulkanCommandBuffer* cmdBuffer)
	{
		for (auto context : m_MainRenderContexts.m_RenderContexts)
		{
			for (auto& renderItem : context->GetRenderObjectsList())
			{
				cmdBuffer->DrawMesh(renderItem, GetCurrentFrame().descriptorAllocator);
			}
		}
	}

	void* VulkanRenderer::GetRenderPassResult(RenderPassType type)
	{
		auto iterator = m_RenderPasses.begin();
		for (; iterator != m_RenderPasses.end(); ++iterator)
		{
			if ((*iterator)->GetType() == type)
			{
				return (*iterator)->GetRenderPassResult();
			}
		}
		return nullptr;
	}

	void VulkanRenderer::AddRenderPass(RenderPassControl* renderPass)
	{
		auto iterator = m_RenderPasses.begin();
		for (; iterator != m_RenderPasses.end(); ++iterator)
		{
			if ((*iterator)->GetPriority() < renderPass->GetPriority())
			{
				break;
			}
		}

		if (iterator == m_RenderPasses.end())
		{
			m_RenderPasses.push_back(renderPass);
		}
		else
		{
			m_RenderPasses.insert(iterator, renderPass);
		}
	}

	void VulkanRenderer::RemoveRenderPass(RenderPassType type)
	{
		for (auto it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it)
		{
			if ((*it)->GetType() == type)
			{
				delete* it;
				m_RenderPasses.erase(it);
				break;
			}
		}
	}

	void VulkanRenderer::AddOffscreenRenderPass(Camera* cam, U32 width, U32 height, bool handleWindowResize, std::function<void(U32, U32)>* onResize)
	{
		if (m_RenderSettings->offscreenRendering)
		{
			AddRenderPass(new VulkanOffscreenRenderPass(cam, this, m_Device, Format::R8G8B8A8_UNORM, Format::D24_UNORM_S8_UINT, width, height));
			if (onResize != nullptr)
			{
				(*onResize) = [this](U32 width, U32 height) {
					m_RenderPassResizeQueries.push_back({ RenderPassType::OffscreenTexture,width,height });
				};
			}
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

		m_ImmediateSubmitData.fence = m_Device->CreateNewFence(true);
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
				m_Device->Release(m_SceneData.buffer[m_CurrentFrame]);
			}
			});
		auto bufferVulkan = Internal_to_Buffer(m_Device->AccessBuffer(m_SceneData.buffer[m_CurrentFrame]));

		m_SceneData.data.ambientColor = Vec4(1, 1, 0, 1);

		SceneData* data = (SceneData*)bufferVulkan->GetMappedData();
		*data = m_SceneData.data;

	}

	void VulkanRenderer::InitImmediateSubmitData()
	{
		m_ImmediateSubmitData.pool = m_Device->CreateCommandPool(false, true);
		m_ImmediateSubmitData.commandBuffer = m_Device->CreateCommandBuffer(true, m_ImmediateSubmitData.pool);
		VulkanCommandBuffer* cmd = (VulkanCommandBuffer*)m_Device->AccessCommandBuffer(m_ImmediateSubmitData.commandBuffer);
	}

	void VulkanRenderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		//handle only additional passes, main render pass is handled by the device
		for (auto it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it)
		{
			if ((*it)->HandleWindowResize())
			{
				(*it)->Resize(width, height);
			}
		}
	}

	void VulkanRenderer::CheckForResizeQueries()
	{
		for (auto it = m_RenderPassResizeQueries.begin(); it != m_RenderPassResizeQueries.end(); ++it)
		{
			auto renderPass = GetRenderPassByType(it->type);
			if (renderPass != nullptr)
			{
				renderPass->Resize(it->width, it->height);
			}
		}
		m_RenderPassResizeQueries.clear();
	}

	RenderPassControl* VulkanRenderer::GetRenderPassByType(RenderPassType type)
	{
		for (auto it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it)
		{
			if ((*it)->GetType() == type)
			{
				return (*it);
			}
		}
		return nullptr;
	}


	void VulkanRenderer::OnPreRenderUpdateContexts(Camera* cam, int width, int height)
	{
		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->FillRenderContexts(cam, this, width, height);
		}

		for (auto context : m_MainRenderContexts.m_RenderContexts)
		{
			auto renderList = context->GetRenderObjectsList();
			for (auto renderElement : renderList)
			{
				renderElement.UpdateSceneDataIfRequired(m_Device, m_SceneData.buffer[m_CurrentFrame]);
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

	void VulkanRenderer::ImmediateSubmitData::Release(VulkanDevice* device)
	{
		device->ReleaseFence(fence);
		device->Release(commandBuffer);
		device->ReleaseCommandPool(pool);
	}

}
