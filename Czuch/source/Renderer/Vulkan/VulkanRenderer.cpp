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
#include"Subsystems/Scenes/Components/CameraComponent.h"

#include"RenderPass/VulkanDepthPrepassRenderPass.h"
#include"RenderPass/VulkanDefaultForwardLightingRenderPass.h"
#include"RenderPass/VulkanDepthLinearPrepassRenderPass.h"
#include"RenderPass/VulkanDefaultForwardTransparentLightingRenderPass.h"
#include"RenderPass/VulkanDebugDrawRenderPass.h"
#include"RenderPass/VulkanFullScreenRenderPass.h"

#include"Events/EventsTypes/ApplicationEvents.h"
#include"EngineRoot.h"


namespace Czuch
{
	VulkanRenderer::VulkanRenderer(Window* window)
	{
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
		EventsManager::Get().RemoveListener(WindowSizeChangedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		m_Device->AwaitDevice();

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_FramesData[a].frameDeletionQueue.Flush();
		}


		delete m_FullScreenRenderPass;
		delete m_FinalRenderPass;

		m_ImmediateSubmitData.Release(m_Device);

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

		bool init = m_Device->InitDevice(&EngineRoot::GetEngineSettings());

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
		InitImmediateSubmitData();

		EventsManager::Get().AddListener(WindowSizeChangedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
	}

	void VulkanRenderer::AfterSystemInit()
	{

	}

	void VulkanRenderer::CreateRenderGraph()
	{
		CreateFrameGraphs();
	}

	void VulkanRenderer::DrawFrame()
	{
		VkDevice device = m_Device->GetNativeDevice();
		auto result = vkWaitForFences(device, 1, &GetCurrentFrame().inFlightFence, VK_TRUE, UINT64_MAX);

		if (result != VK_SUCCESS)
		{
			LOG_BE_ERROR("[Vulkan]Failed to wait for fence: {0}", VkResultToString(result));
			return;
		}

		GetCurrentFrame().Reset();
		GetCurrentFrame().frameDeletionQueue.Flush();

		m_Device->PreDrawFrame();

		bool failedToAcquire = false;
		uint32_t imageIndex = m_Device->AcquireNextSwapChainImage(GetCurrentFrame().imageAvailableSemaphore, failedToAcquire);
		if (failedToAcquire)
		{
			CZUCH_BE_ASSERT(false, "Failed to aquire next swap chain image.");
			LOG_BE_ERROR("[Vulkan]Failed to acquire next swap chain image.");
			return;
		}

		BeforeFrameGraphExecute();
		vkResetFences(device, 1, &GetCurrentFrame().inFlightFence);

		auto cmdBuffer = m_Device->AccessCommandBuffer(GetCurrentFrame().commandBuffer);
		cmdBuffer->Begin();

		if (m_ActiveScene->GetActiveFrameGraphsCount() > 0)
		{

			auto mainFrameGraph = m_ActiveScene->GetFrameGraphControl(0);
			mainFrameGraph->Execute(m_Device, cmdBuffer);
			auto finalInfo = mainFrameGraph->GetFinalFrameGraphNodeInfo();
			m_FullScreenRenderPass->SetSourceTexture(cmdBuffer, finalInfo.finalTexture, finalInfo.finalDepthTexture, finalInfo.finalRenderPass);

			for (int a = 1; a < m_ActiveScene->GetActiveFrameGraphsCount(); ++a)
			{
				auto* frameGraph = m_ActiveScene->GetFrameGraphControl(a);

				frameGraph->Execute(m_Device, cmdBuffer);

				auto finalCurrentInfo = frameGraph->GetFinalFrameGraphNodeInfo();
				m_FullScreenRenderPass->SetViewportAndScissor(frameGraph->GetCamera());
				m_FullScreenRenderPass->SetTargetTexture(cmdBuffer, finalCurrentInfo.finalTexture);
				m_FullScreenRenderPass->PreDraw(cmdBuffer, this);
				m_FullScreenRenderPass->Execute(cmdBuffer);
				m_FullScreenRenderPass->PostDraw(cmdBuffer, this);
			}

			m_FinalRenderPass->SetViewportAndScissor(mainFrameGraph->GetCamera());
			m_FinalRenderPass->SetFinalTexture(cmdBuffer, finalInfo.finalTexture);
			m_FinalRenderPass->PreDraw(cmdBuffer, this);
			m_FinalRenderPass->Execute(cmdBuffer);
			m_FinalRenderPass->PostDraw(cmdBuffer, this);
		}


		cmdBuffer->End();

		SubmitCommandBuffer();
		m_Device->Present(imageIndex, GetCurrentFrame().renderFinishedSemaphote);

		AfterFrameGraphExecute();

		CheckForResizeQueries();

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::AwaitDeviceIdle()
	{
		m_Device->AwaitDevice();
	}

	void VulkanRenderer::ReleaseDependencies()
	{
		m_FullScreenRenderPass->Release();
		m_FinalRenderPass->Release();
	}

	GraphicsDevice* VulkanRenderer::GetDevice()
	{
		return static_cast<GraphicsDevice*>(m_Device);
	}


	void VulkanRenderer::SetActiveScene(Scene* scene)
	{
		if (m_ActiveScene == scene)
		{
			return;
		}
		m_ActiveScene = scene;
		scene->OnSceneActive(this, m_Device);
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

	void VulkanRenderer::DrawScene(VulkanCommandBuffer* cmdBuffer, Camera* camera, RenderContextFillParams* params)
	{
		if (m_ActiveScene == nullptr)
		{
			return;
		}

		auto context = m_ActiveScene->GetRenderContext(params->renderPassType, camera);

		if (context == nullptr)
		{
			return;
		}

		for (auto& renderItem : context->GetRenderObjectsList())
		{
			cmdBuffer->DrawMesh(renderItem, GetCurrentFrame().descriptorAllocator);
		}

	}

	void VulkanRenderer::DrawDebugData(VulkanCommandBuffer* cmdBuffer, Camera* camera, RenderContextFillParams* params)
	{
		if (m_ActiveScene == nullptr)
		{
			return;
		}

		auto& camerasControl = m_ActiveScene->GetCamerasControl();

		for (auto& cameraControl : camerasControl)
		{
			if (cameraControl.camera == camera || camera == nullptr)
			{
				auto& data = cameraControl.GetIndirectDrawDataForDebugDrawingLines(params[0], m_CurrentFrame);
				cmdBuffer->DrawIndirectIndexedWithData(&data, GetCurrentFrame().descriptorAllocator);
				auto& dataTris = cameraControl.GetIndirectDrawDataForDebugDrawingTriangles(params[1], m_CurrentFrame);
				cmdBuffer->DrawIndirectIndexedWithData(&dataTris, GetCurrentFrame().descriptorAllocator);
				auto& dataPoints = cameraControl.GetIndirectDrawDataForDebugDrawingPoints(params[2], m_CurrentFrame);
				cmdBuffer->DrawIndirectIndexedWithData(&dataPoints, GetCurrentFrame().descriptorAllocator);
				return;
			}
		}

		LOG_BE_ERROR("[Vulkan]DrawDebugData: Camera not found in scene cameras control.");
	}

	void VulkanRenderer::DrawFullScreenQuad(VulkanCommandBuffer* cmdBuffer, MaterialInstanceHandle material)
	{
		cmdBuffer->DrawFullScreenQuad(material, GetCurrentFrame().descriptorAllocator);
	}

	void* VulkanRenderer::GetRenderPassResult(Camera* cam, RenderPassType type)
	{
		CZUCH_BE_ASSERT(m_ActiveScene != nullptr, "Active scene is null, cannot get render pass result.");

		auto frameGraph = m_ActiveScene->GetFrameGraphControl(cam);
		if (frameGraph == nullptr)
		{
			LOG_BE_ERROR("[Vulkan]GetRenderPassResult: Frame graph is null for camera.");
			return nullptr;
		}

		return frameGraph->GetRenderPassControlByType(type);
	}

	bool VulkanRenderer::HasRenderPass(Camera* cam, RenderPassType type)
	{
		CZUCH_BE_ASSERT(m_ActiveScene != nullptr, "Active scene is null, cannot check render pass existence.");
		auto frameGraph = m_ActiveScene->GetFrameGraphControl(cam);
		if (frameGraph == nullptr)
		{
			LOG_BE_ERROR("[Vulkan]HasRenderPass: Frame graph is null for camera.");
			return false;
		}
		return frameGraph->HasRenderPass(type);
	}

	void VulkanRenderer::FlushDeletionQueue()
	{
		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_FramesData[a].frameDeletionQueue.Flush();
		}
	}


	void VulkanRenderer::RegisterRenderPassResizeEventResponse(U32 width, U32 height, bool handleAll, std::function<void(U32, U32)>* onResize)
	{
		if (onResize != nullptr)
		{
			(*onResize) = [this, handleAll](U32 width, U32 height) {
				m_RenderPassResizeQueries.push_back({ RenderPassType::OffscreenTexture,handleAll,width,height });
				};
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

	void VulkanRenderer::BeforeFrameGraphExecute()
	{
		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->BeforeFrameGraphExecute(GetCurrentFrame().commandBuffer, m_CurrentFrame, m_FramesData[m_CurrentFrame].frameDeletionQueue);
		}
	}

	void VulkanRenderer::AfterFrameGraphExecute()
	{
		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->AfterFrameGraphExecute(GetCurrentFrame().commandBuffer);
		}
	}

	void VulkanRenderer::InitImmediateSubmitData()
	{
		m_ImmediateSubmitData.pool = m_Device->CreateCommandPool(false, true);
		m_ImmediateSubmitData.commandBuffer = m_Device->CreateCommandBuffer(true, m_ImmediateSubmitData.pool);
		VulkanCommandBuffer* cmd = (VulkanCommandBuffer*)m_Device->AccessCommandBuffer(m_ImmediateSubmitData.commandBuffer);
	}

	void VulkanRenderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->OnResize(width, height, true);
		};
		m_FinalRenderPass->Resize(width, height);
		m_FullScreenRenderPass->Resize(width, height);
	}

	void VulkanRenderer::CheckForResizeQueries()
	{
		bool resize = false;
		for (auto it = m_RenderPassResizeQueries.begin(); it != m_RenderPassResizeQueries.end(); ++it)
		{
			if (it->allNotHandledByWindowSizeChanged)
			{
				m_Device->AwaitDevice();
				if (m_ActiveScene != nullptr)
				{
					m_ActiveScene->OnResize(it->width, it->height, false);
				}
			}
			else
			{
				if (m_ActiveScene != nullptr)
				{
					m_ActiveScene->OnResizeRenderPassType(it->type, it->width, it->height);
				}
			}
		}
		m_RenderPassResizeQueries.clear();
	}

	void VulkanRenderer::OnDebugRenderingFlagsChanged(U32 oldFlags, bool forceSetCurrentFlag)
	{
		auto activeScene = m_ActiveScene;

		if (activeScene == nullptr)
		{
			LOG_BE_ERROR("[Vulkan]OnDebugRenderingFlagsChanged: Active scene is null, cannot update debug rendering flags.");
			return;
		}

		auto currentFlags = CurrentDebugRenderingFlags();

		auto wantMaterialIndexEnabled = (currentFlags & DebugRenderingFlag::MaterialIndexAsColor) != 0;
		auto currentlyMaterialIndexEnabled = (oldFlags & DebugRenderingFlag::MaterialIndexAsColor) != 0;

		if (wantMaterialIndexEnabled != currentlyMaterialIndexEnabled || forceSetCurrentFlag)
		{
			activeScene->ForEachFrameGraph([wantMaterialIndexEnabled](FrameGraphControl* frameGraph) {
				if (frameGraph != nullptr)
				{
					frameGraph->SetDebugRenderFlag(DebugRenderingFlag::MaterialIndexAsColor, wantMaterialIndexEnabled);
				}
				});
		}

	}

	void VulkanRenderer::OnPreRenderUpdateContexts(Camera* cam, int width, int height, RenderContextFillParams* fillParams)
	{
		if (fillParams == nullptr)
		{
			fillParams = &m_DefaultContextFillParams;
		}

		if (m_ActiveScene != nullptr)
		{
			RenderContext* ctx = m_ActiveScene->FillRenderContexts(cam, this, width, height, *fillParams);

			auto& renderList = ctx->GetRenderObjectsList();
			auto sceneDataBuffers = m_ActiveScene->GetSceneDataBuffers(cam, m_CurrentFrame, fillParams->renderPassType);
			for (auto& renderElement : renderList)
			{
				renderElement.UpdateSceneDataIfRequired(m_Device, sceneDataBuffers, *fillParams);
			}
		}

	}

	void VulkanRenderer::OnPostRenderUpdateContexts(Camera* camera, RenderContextFillParams* fillParams)
	{
		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->OnPostRender(camera, fillParams);
		}
	}

	void VulkanRenderer::OnPreRenderUpdateDebugDrawElements(Camera* cam, RenderContextFillParams* fillParams)
	{
		if (fillParams == nullptr)
		{
			fillParams = &m_DefaultContextFillParams;
		}

		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->FillDebugDrawElements(cam, this, *fillParams);
		}

	}

	void* VulkanRenderer::GetFrameGraphFinalResult(Camera* cam)
	{
		CZUCH_BE_ASSERT(m_ActiveScene != nullptr, "Active scene is null, cannot get frame graph final result.");
		auto frameGraph = m_ActiveScene->GetFrameGraphControl(cam);
		if (frameGraph == nullptr)
		{
			LOG_BE_ERROR("[Vulkan]GetFrameGraphFinalResult: Frame graph is null for camera.");
			return nullptr;
		}
		return frameGraph->GetFrameGraphFinalResult();
	}

	void VulkanRenderer::CreateFrameGraphs()
	{
		//create main render pass control
		m_FinalRenderPass = new VulkanMainRenderPass(m_Device, this);
		m_FinalRenderPass->SetNativeRenderPassHandle(m_Device->GetSwapChainRenderPass());

		m_FullScreenRenderPass = new VulkanFullScreenRenderPass(m_Device, this);
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
