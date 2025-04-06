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

#include"Events/EventsTypes/ApplicationEvents.h"


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
		EventsManager::Get().RemoveListener(WindowSizeChangedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
		m_Device->AwaitDevice();

		m_SceneData.Release(m_Device);

		ReleaseFrameGraphs();

		UnRegisterRenderPassControl(m_FinalRenderPass);
		delete m_FinalRenderPass;

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
		InitImmediateSubmitData();

		EventsManager::Get().AddListener(WindowSizeChangedEvent::GetStaticEventType(), (Czuch::IEventsListener*)this);
	}

	void VulkanRenderer::AfterSystemInit()
	{
		m_CurrentFrameGraph.AfterSystemInit();
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

		SetSceneData();
		vkResetFences(device, 1, &GetCurrentFrame().inFlightFence);

		auto cmdBuffer = m_Device->AccessCommandBuffer(GetCurrentFrame().commandBuffer);
		cmdBuffer->Begin();
		//render passes from frame graph
		m_CurrentFrameGraph.Execute(m_Device, cmdBuffer);

		auto finalTexture = m_CurrentFrameGraph.GetFinalTexture();
		m_FinalRenderPass->SetFinalTexture(cmdBuffer, finalTexture);
		m_FinalRenderPass->PreDraw(cmdBuffer, this);
		m_FinalRenderPass->Execute(cmdBuffer);
		m_FinalRenderPass->PostDraw(cmdBuffer, this);

		cmdBuffer->End();

		SubmitCommandBuffer();
		m_Device->Present(imageIndex, GetCurrentFrame().renderFinishedSemaphote);

		CheckForResizeQueries();

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::AwaitDeviceIdle()
	{
		m_Device->AwaitDevice();
	}

	void VulkanRenderer::ReleaseDependencies()
	{
		m_CurrentFrameGraph.ReleaseDependencies();
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
		InitSceneData();
		m_CurrentFrameGraph.SetClearColor(m_ActiveScene->GetClearColor());

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

	void VulkanRenderer::DrawScene(VulkanCommandBuffer* cmdBuffer, RenderContext* context)
	{
		for (auto& renderItem : context->GetRenderObjectsList())
		{
			cmdBuffer->DrawMesh(renderItem, GetCurrentFrame().descriptorAllocator);
		}

	}

	void VulkanRenderer::DrawFullScreenQuad(VulkanCommandBuffer* cmdBuffer, MaterialInstanceHandle material)
	{
		cmdBuffer->DrawFullScreenQuad(material, GetCurrentFrame().descriptorAllocator);
	}

	void* VulkanRenderer::GetRenderPassResult(RenderPassType type)
	{
		return m_CurrentFrameGraph.GetRenderPassResult(type);
	}

	bool VulkanRenderer::HasRenderPass(RenderPassType type)
	{
		return m_CurrentFrameGraph.HasRenderPass(type);
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

	void VulkanRenderer::InitSceneData()
	{

		m_SceneData.bufferDesc.persistentMapped = true;
		m_SceneData.bufferDesc.elementsCount = 1;
		m_SceneData.bufferDesc.bind_flags = BindFlag::UNIFORM_BUFFER;
		m_SceneData.bufferDesc.size = sizeof(SceneData);
		m_SceneData.bufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		m_SceneData.lightsBufferDesc.persistentMapped = true;
		m_SceneData.lightsBufferDesc.elementsCount = MAX_LIGHTS_IN_SCENE;
		m_SceneData.lightsBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		m_SceneData.lightsBufferDesc.size = sizeof(LightData) * MAX_LIGHTS_IN_SCENE;
		m_SceneData.lightsBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		m_SceneData.InitTilesBuffer(m_Device, false, 0, 0);

		m_SceneData.data.ambientColor = Vec4(1, 1, 1, 1);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_Device->Release(m_SceneData.buffer[a]);
			INVALIDATE_HANDLE(m_SceneData.buffer[a]);
			m_SceneData.lightsBuffer[a] = m_Device->CreateBuffer(&m_SceneData.lightsBufferDesc);
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

		if (m_ActiveScene != nullptr)
		{
			m_SceneData.data.ambientColor = m_ActiveScene->GetAmbientColor();

			auto& lights = m_ActiveScene->GetAllLightObjects();
			if (m_SceneData.FillTilesWithLights(m_Device, lights, m_CurrentFrame))
			{
				m_SceneData.UpdateMaterialsLightsInfo();
			}
		}

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
		m_SceneData.InitTilesBuffer(m_Device, true, width, height);
		m_CurrentFrameGraph.ResizeRenderPasses(width, height, true);
		m_FinalRenderPass->Resize(width, height);
	}

	void VulkanRenderer::CheckForResizeQueries()
	{
		bool resize = false;
		for (auto it = m_RenderPassResizeQueries.begin(); it != m_RenderPassResizeQueries.end(); ++it)
		{
			if (it->allNotHandledByWindowSizeChanged)
			{
				m_Device->AwaitDevice();
				m_CurrentFrameGraph.ResizeRenderPasses(it->width, it->height, false);
			}
			else
			{
				auto renderPass = GetRenderPassByType(it->type);
				if (renderPass != nullptr)
				{
					renderPass->Resize(it->width, it->height);
				}
			}
		}
		m_RenderPassResizeQueries.clear();
	}


	RenderPassControl* VulkanRenderer::GetRenderPassByType(RenderPassType type)
	{
		return m_CurrentFrameGraph.GetRenderPassControlByType(type);
	}


	void VulkanRenderer::OnPreRenderUpdateContexts(Camera* cam, int width, int height, RenderContextFillParams* fillParams, RenderContext* mainRenderContext)
	{
		if (fillParams == nullptr)
		{
			fillParams = &m_DefaultContextFillParams;
		}

		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->FillRenderContexts(cam, this, width, height, *fillParams, mainRenderContext);
		}


		auto& renderList = mainRenderContext->GetRenderObjectsList();
		auto sceneDataBuffers = m_SceneData.GetSceneDataBuffers(m_CurrentFrame);
		for (auto& renderElement : renderList)
		{
			renderElement.UpdateSceneDataIfRequired(m_Device, sceneDataBuffers, *fillParams);
		}

	}

	void VulkanRenderer::OnPostRenderUpdateContexts(RenderContextFillParams* fillParams, RenderContext* mainRenderContext)
	{
		if (mainRenderContext->IsAutoCleanEnabled())
		{
			mainRenderContext->ClearRenderList();
		}
	}

	RenderPassControl* VulkanRenderer::RegisterRenderPassControl(RenderPassControl* control)
	{
		m_RenderPassControls.push_back(control);
		return control;
	}

	void VulkanRenderer::UnRegisterRenderPassControl(RenderPassControl* control)
	{
		auto it = std::find(m_RenderPassControls.begin(), m_RenderPassControls.end(), control);
		if (it != m_RenderPassControls.end())
		{
			m_RenderPassControls.erase(it);
		}
	}

	RenderPassHandle VulkanRenderer::GetNativeRenderPassHandle(RenderPassType type)
	{
		for (auto control : m_RenderPassControls)
		{
			if (control->GetType() == type)
			{
				return control->GetNativeRenderPassHandle();
			}
		}
		return INVALID_HANDLE(RenderPassHandle);
	}

	void* VulkanRenderer::GetFrameGraphFinalResult()
	{
		return m_CurrentFrameGraph.GetFinalRenderPassResult();
	}

	void VulkanRenderer::CreateFrameGraphs()
	{
		//here we will use frame graph builder to create frame graphs
		//and we will take info of what kind of graph we need from render settings
		//or use can provide his own frame graph in the future
		m_FrameGraphBuilder.Init(m_Device, this);

		//create basic frame graph with main render pass and depth prepass


		//Color node

		/////////////DOFPass
		/*FrameGraphResourceInputCreation dofDepthInput;
		dofDepthInput.name = "Depth";
		dofDepthInput.type = FrameGraphResourceType::Reference;

		FrameGraphResourceInputCreation dofLightingInput;
		dofLightingInput.name = "Lighting";
		dofLightingInput.type = FrameGraphResourceType::Reference;


		FrameGraphResourceOutputCreation mainRenderPassOutput;
		mainRenderPassOutput.name = "Final";
		mainRenderPassOutput.type = FrameGraphResourceType::Attachment;
		mainRenderPassOutput.resource_info.texture.format = Format::R8G8B8A8_UNORM;
		mainRenderPassOutput.resource_info.texture.width = m_Device->GetSwapchainWidth();
		mainRenderPassOutput.resource_info.texture.height = m_Device->GetSwapchainHeight();
		mainRenderPassOutput.resource_info.texture.depth = 1;
		mainRenderPassOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;


		m_FrameGraphBuilder.BeginNewNode("DOFPass");
		m_FrameGraphBuilder.AddInput(dofDepthInput);
		m_FrameGraphBuilder.AddInput(dofLightingInput);
		m_FrameGraphBuilder.SetRenderPassControl(nullptr);
		m_FrameGraphBuilder.AddOutput(mainRenderPassOutput);
		m_FrameGraphBuilder.EndNode();
		/////////////////////////

		///////////////Depth node
		FrameGraphResourceOutputCreation depthPrepassOutput;
		depthPrepassOutput.name = "Depth";
		depthPrepassOutput.type = FrameGraphResourceType::Attachment;
		depthPrepassOutput.resource_info.texture.format = Format::D24_UNORM_S8_UINT;
		depthPrepassOutput.resource_info.texture.width = m_Device->GetSwapchainWidth();
		depthPrepassOutput.resource_info.texture.height = m_Device->GetSwapchainHeight();
		depthPrepassOutput.resource_info.texture.depth = 1;
		depthPrepassOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;

		m_FrameGraphBuilder.BeginNewNode("DepthPrepass");
		m_FrameGraphBuilder.AddOutput(depthPrepassOutput);
		m_FrameGraphBuilder.EndNode();
		//////////////////////////

		///////////////GBuffer pass
		FrameGraphResourceInputCreation gBufferDepthInput;
		gBufferDepthInput.name = "Depth";
		gBufferDepthInput.type = FrameGraphResourceType::Attachment;

		FrameGraphResourceOutputCreation gBufferOutput;
		gBufferOutput.name = "GBuffer_Color";
		gBufferOutput.type = FrameGraphResourceType::Attachment;
		gBufferOutput.resource_info.texture.format = Format::R8G8B8A8_UNORM;
		gBufferOutput.resource_info.texture.width = m_Device->GetSwapchainWidth();
		gBufferOutput.resource_info.texture.height = m_Device->GetSwapchainHeight();
		gBufferOutput.resource_info.texture.depth = 1;
		gBufferOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;

		FrameGraphResourceOutputCreation gBufferNormalOutput;
		gBufferNormalOutput.name = "GBuffer_Normal";
		gBufferNormalOutput.type = FrameGraphResourceType::Attachment;
		gBufferNormalOutput.resource_info.texture.format = Format::R16G16B16A16_FLOAT;
		gBufferNormalOutput.resource_info.texture.width = m_Device->GetSwapchainWidth();
		gBufferNormalOutput.resource_info.texture.height = m_Device->GetSwapchainHeight();
		gBufferNormalOutput.resource_info.texture.depth = 1;
		gBufferNormalOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;

		FrameGraphResourceOutputCreation gBufferPositionOutput;
		gBufferPositionOutput.name = "GBuffer_Position";
		gBufferPositionOutput.type = FrameGraphResourceType::Attachment;
		gBufferPositionOutput.resource_info.texture.format = Format::R16G16B16A16_FLOAT;
		gBufferPositionOutput.resource_info.texture.width = m_Device->GetSwapchainWidth();
		gBufferPositionOutput.resource_info.texture.height = m_Device->GetSwapchainHeight();
		gBufferPositionOutput.resource_info.texture.depth = 1;

		m_FrameGraphBuilder.BeginNewNode("GBufferPass");
		m_FrameGraphBuilder.AddInput(gBufferDepthInput);
		m_FrameGraphBuilder.AddOutput(gBufferOutput);
		m_FrameGraphBuilder.AddOutput(gBufferNormalOutput);
		m_FrameGraphBuilder.AddOutput(gBufferPositionOutput);
		m_FrameGraphBuilder.EndNode();
		//////////////////////////

		///////////////Lighting pass
		FrameGraphResourceInputCreation colorInput;
		colorInput.name = "GBuffer_Color";
		colorInput.type = FrameGraphResourceType::Texture;

		FrameGraphResourceInputCreation normalInput;
		normalInput.name = "GBuffer_Normal";
		normalInput.type = FrameGraphResourceType::Texture;

		FrameGraphResourceInputCreation positionInput;
		positionInput.name = "GBuffer_Position";
		positionInput.type = FrameGraphResourceType::Texture;

		FrameGraphResourceOutputCreation lightingOutput;
		lightingOutput.name = "Lighting";
		lightingOutput.type = FrameGraphResourceType::Attachment;
		lightingOutput.resource_info.texture.format = Format::R8G8B8A8_UNORM;
		lightingOutput.resource_info.texture.width = m_Device->GetSwapchainWidth();
		lightingOutput.resource_info.texture.height = m_Device->GetSwapchainHeight();
		lightingOutput.resource_info.texture.depth = 1;
		lightingOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;

		m_FrameGraphBuilder.BeginNewNode("LightingPass");
		m_FrameGraphBuilder.AddInput(colorInput);
		m_FrameGraphBuilder.AddInput(normalInput);
		m_FrameGraphBuilder.AddInput(positionInput);
		m_FrameGraphBuilder.AddOutput(lightingOutput);
		m_FrameGraphBuilder.EndNode();
		//////////////////////////

		///////////////Lighting transparent pass
		FrameGraphResourceInputCreation lightInputTransparent;
		lightInputTransparent.name = "Lighting";
		lightInputTransparent.type = FrameGraphResourceType::Attachment;

		FrameGraphResourceInputCreation depthInputTransparent;
		depthInputTransparent.name = "Depth";
		depthInputTransparent.type = FrameGraphResourceType::Attachment;

		FrameGraphResourceOutputCreation lightingOutputTransparent;
		lightingOutputTransparent.name = "Lighting";
		lightingOutputTransparent.type = FrameGraphResourceType::Reference;

		m_FrameGraphBuilder.BeginNewNode("LightingTransparentPass");
		m_FrameGraphBuilder.AddInput(lightInputTransparent);
		m_FrameGraphBuilder.AddInput(depthInputTransparent);
		m_FrameGraphBuilder.AddOutput(lightingOutputTransparent);
		m_FrameGraphBuilder.EndNode();
		//////////////////////////

		m_FrameGraphBuilder.Build(m_CurrentFrameGraph);*/

		bool handleWindowResize = !m_RenderSettings->RenderingTargetSizeExternallySet();
		U32 startWidth = handleWindowResize ? m_Device->GetSwapchainWidth() : m_RenderSettings->targetWidth;
		U32 startHeight = handleWindowResize ? m_Device->GetSwapchainHeight() : m_RenderSettings->targetHeight;

		///////////////Depth node
		FrameGraphResourceOutputCreation depthPrepassOutput;
		depthPrepassOutput.name = "Depth";
		depthPrepassOutput.type = FrameGraphResourceType::Attachment;
		depthPrepassOutput.resource_info.texture.format = ConvertVkFormat(m_Device->GetDepthFormat());
		depthPrepassOutput.resource_info.texture.width = startWidth;
		depthPrepassOutput.resource_info.texture.height = startHeight;
		depthPrepassOutput.resource_info.texture.depth = 1;
		depthPrepassOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;
		depthPrepassOutput.resource_info.texture.usage = ImageUsageFlag::DEPTH_STENCIL_ATTACHMENT;

		m_FrameGraphBuilder.BeginNewNode("DepthPrepass");
		m_FrameGraphBuilder.AddOutput(depthPrepassOutput);
		m_FrameGraphBuilder.SetRenderPassControl(RegisterRenderPassControl(new VulkanDepthPrepassRenderPass(this, m_Device, startWidth, startHeight, handleWindowResize)));
		m_FrameGraphBuilder.SetClearColor(Vec3(0.0f));
		m_FrameGraphBuilder.EndNode();
		//////////////////////////

		///////////////Depth to linear pass

		FrameGraphResourceInputCreation depthAsTextureInput;
		depthAsTextureInput.type = FrameGraphResourceType::Texture;
		depthAsTextureInput.name = "Depth";

		FrameGraphResourceOutputCreation depthLinearPrepassOutput;
		depthLinearPrepassOutput.name = "DepthLinear";
		depthLinearPrepassOutput.type = FrameGraphResourceType::Attachment;
		depthLinearPrepassOutput.resource_info.texture.format = Format::R8G8B8A8_UNORM;
		depthLinearPrepassOutput.resource_info.texture.width = startWidth;
		depthLinearPrepassOutput.resource_info.texture.height = startHeight;
		depthLinearPrepassOutput.resource_info.texture.depth = 1;
		depthLinearPrepassOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;
		depthLinearPrepassOutput.resource_info.texture.usage = ImageUsageFlag::COLOR_ATTACHMENT;

		m_FrameGraphBuilder.BeginNewNode("DepthLinearPrepass");
		m_FrameGraphBuilder.AddInput(depthAsTextureInput);
		m_FrameGraphBuilder.SetClearColor(Vec3(0.0f, 0.0f, 1.0f));
		m_FrameGraphBuilder.AddOutput(depthLinearPrepassOutput);
		m_FrameGraphBuilder.SetRenderPassControl(RegisterRenderPassControl(new VulkanDepthLinearPrepassRenderPass(this, m_Device, startWidth, startHeight, handleWindowResize)));
		m_FrameGraphBuilder.EndNode();
		/////////////////////////////


		///////////////Lighting pass

		FrameGraphResourceInputCreation depthInput;
		depthInput.type = FrameGraphResourceType::Attachment;
		depthInput.name = "Depth";

		FrameGraphResourceOutputCreation lightingOutput;
		lightingOutput.name = "Lighting";
		lightingOutput.type = FrameGraphResourceType::Attachment;
		lightingOutput.resource_info.texture.format = Format::R8G8B8A8_UNORM;
		lightingOutput.resource_info.texture.width = startWidth;
		lightingOutput.resource_info.texture.height = startHeight;
		lightingOutput.resource_info.texture.depth = 1;
		lightingOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;
		lightingOutput.resource_info.texture.usage = ImageUsageFlag::COLOR_ATTACHMENT;

		auto lightingPass = new VulkanDefaultForwardLightingRenderPass(this, m_Device, startWidth, startHeight, handleWindowResize);
		m_FrameGraphBuilder.BeginNewNode("LightingPass");
		m_FrameGraphBuilder.AddInput(depthInput);
		m_FrameGraphBuilder.AddOutput(lightingOutput);
		m_FrameGraphBuilder.SetClearColor(Vec3(0.0f, 0.0f, 0.0f));
		m_FrameGraphBuilder.SetRenderPassControl(RegisterRenderPassControl(lightingPass));
		m_FrameGraphBuilder.EndNode();
		//////////////////////////

		///////////////Transparent Lighting pass

		FrameGraphResourceInputCreation depthInputTransparent;
		depthInputTransparent.type = FrameGraphResourceType::Attachment;
		depthInputTransparent.name = "Depth";

		FrameGraphResourceInputCreation lightingInputTransparent;
		lightingInputTransparent.type = FrameGraphResourceType::Attachment;
		lightingInputTransparent.name = "Lighting";


		auto lightingPassTransparent = new VulkanDefaultForwardTransparentLightingRenderPass(this, m_Device, startWidth, startHeight, handleWindowResize);

		m_FrameGraphBuilder.BeginNewNode("LightingPassTransparent");
		m_FrameGraphBuilder.AddInput(depthInputTransparent);
		m_FrameGraphBuilder.AddInput(lightingInputTransparent);
		m_FrameGraphBuilder.SetRenderPassControl(RegisterRenderPassControl(lightingPassTransparent));
		m_FrameGraphBuilder.EndNode();


		//////////////////

		/////////////////////////Debug Render Pass

		FrameGraphResourceInputCreation depthInputDebug;
		depthInputDebug.type = FrameGraphResourceType::Attachment;
		depthInputDebug.name = "Depth";

		FrameGraphResourceInputCreation lightingInputDebug;
		lightingInputDebug.type = FrameGraphResourceType::Attachment;
		lightingInputDebug.name = "Lighting";

		auto debugDrawPass = new VulkanDebugDrawRenderPass(this, m_Device, startWidth, startHeight, handleWindowResize);

		m_FrameGraphBuilder.BeginNewNode("DebugDrawPass");
		m_FrameGraphBuilder.AddInput(depthInputDebug);
		m_FrameGraphBuilder.AddInput(lightingInputDebug);
		m_FrameGraphBuilder.SetRenderPassControl(RegisterRenderPassControl(debugDrawPass));
		m_FrameGraphBuilder.EndNode();



		////////////////

		m_CurrentFrameGraph.Init(m_Device, this);
		m_FrameGraphBuilder.Build(&m_CurrentFrameGraph);

		//create main render pass control
		m_FinalRenderPass = new VulkanMainRenderPass(m_Device, this);
		m_FinalRenderPass->SetNativeRenderPassHandle(m_Device->GetSwapChainRenderPass());
		RegisterRenderPassControl(m_FinalRenderPass);
	}

	void VulkanRenderer::ReleaseFrameGraphs()
	{
		m_FrameGraphBuilder.Release();
		m_CurrentFrameGraph.Release();
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

	void VulkanRenderer::SceneDataContainer::Init(VulkanDevice* device)
	{
		lightsIndexList.reserve(MAX_LIGHTS_IN_SCENE * MAX_LIGHTS_IN_SCENE);
		tilesDataContainer.tilesData.reserve(MAX_LIGHTS_IN_SCENE * MAX_LIGHTS_IN_SCENE);
		lightsData.reserve(MAX_LIGHTS_IN_SCENE);
	}

	void VulkanRenderer::SceneDataContainer::Release(VulkanDevice* device)
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			device->Release(tilesBuffer[i]);
			device->Release(lightsListBuffer[i]);
			device->Release(lightsBuffer[i]);
		}
	}

	void VulkanRenderer::SceneDataContainer::InitTilesBuffer(VulkanDevice* device, bool resize, U32 width, U32 height)
	{
		if (width == 0 || height == 0)
		{
			width = device->GetSwapchainWidth();
			height = device->GetSwapchainHeight();
		}

		tiles_in_width = ((width + TILE_SIZE - 1) / TILE_SIZE);
		tiles_in_height = ((height + TILE_SIZE - 1) / TILE_SIZE);


		tiles_count = tiles_in_width * tiles_in_height;

		tilesBufferDesc.persistentMapped = true;
		tilesBufferDesc.elementsCount = tiles_count;
		tilesBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		tilesBufferDesc.size = sizeof(LightsTileData) * tiles_count + 4 * sizeof(U32);
		tilesBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		lightsListBufferDesc.persistentMapped = true;
		lightsListBufferDesc.elementsCount = MAX_LIGHTS_IN_SCENE * tiles_count;
		lightsListBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		lightsListBufferDesc.size = sizeof(U32) * MAX_LIGHTS_IN_SCENE * tiles_count;
		lightsListBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			device->Release(tilesBuffer[i]);
			device->Release(lightsListBuffer[i]);
		}


		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			tilesBuffer[i] = device->CreateBuffer(&tilesBufferDesc);
			lightsListBuffer[i] = device->CreateBuffer(&lightsListBufferDesc);
		}

		tilesDataContainer.screenSize = glm::ivec4(width, height, tiles_in_width, tiles_in_height);

		//update materials for elements size?
		UpdateMaterialsLightsInfo();
	}

	bool VulkanRenderer::SceneDataContainer::FillTilesWithLights(VulkanDevice* device, const Array<LightObjectInfo>& allLights, U32 frame)
	{
		bool lightsChanged = false;
		lightsIndexList.clear();
		tilesDataContainer.tilesData.clear();
		lightsData.clear();

		//fill lights data
		auto bufferVulkanLights = Internal_to_Buffer(device->AccessBuffer(lightsBuffer[frame]));
		LightData* lightsData = (LightData*)bufferVulkanLights->GetMappedData();
		for (int a = 0; a < allLights.size(); ++a)
		{
			lightsData[a] = allLights[a].lightData;
		}

		//we want to check if number of lights changed so we know if we need to update material info about lights
		if (lastLightsCount != allLights.size())
		{
			lightsChanged = true;
			lastLightsCount = allLights.size();
		}

		for (int i = 0; i < tiles_count; ++i)
		{
			//here compute visible lights for each tile
			//and store them in tilesData

			//for now we will just store all lights in each tile
			int startIndex = lightsIndexList.size();
			U32 count = allLights.size() <= MAX_LIGHTS_IN_TILE ? allLights.size() : MAX_LIGHTS_IN_TILE;
			for (int a = 0; a < count; ++a)
			{
				lightsIndexList.push_back(a);
			}
			int endIndex = lightsIndexList.size() - 1;

			LightsTileData tileData;
			tileData.lightStart = startIndex;
			tileData.lightCount = endIndex - startIndex + 1;
			tilesDataContainer.tilesData.emplace_back(tileData);
		}

		//fill tiled data buffer with data
		auto bufferVulkan = Internal_to_Buffer(device->AccessBuffer(tilesBuffer[frame]));
		void* data = bufferVulkan->GetMappedData();
		uint8_t* byteData = static_cast<uint8_t*>(data);

		// Copy screenSize at the start
		memcpy(byteData, &tilesDataContainer.screenSize, sizeof(glm::ivec4));

		// Move the pointer forward by sizeof(glm::ivec4)
		byteData += sizeof(glm::ivec4);

		// Copy tile data after screenSize
		memcpy(byteData, tilesDataContainer.tilesData.data(), sizeof(LightsTileData) * tilesDataContainer.tilesData.size());

		//fill lights list buffer
		Buffer_Vulkan* bufferVulkanLightsList = Internal_to_Buffer(device->AccessBuffer(lightsListBuffer[frame]));
		U32* lightsListData = (U32*)bufferVulkanLightsList->GetMappedData();
		//do memcopy
		memcpy(lightsListData, lightsIndexList.data(), sizeof(U32) * lightsIndexList.size());
		return lightsChanged;
	}

	void VulkanRenderer::SceneDataContainer::UpdateMaterialsLightsInfo()
	{
		AssetsManager::Get().UpdateLightingMaterialsLightInfo(MAX_LIGHTS_IN_SCENE, MAX_LIGHTS_IN_TILE * tiles_count, tiles_count);
	}

	SceneDataBuffers VulkanRenderer::SceneDataContainer::GetSceneDataBuffers(U32 frame)
	{
		SceneDataBuffers buffers;
		buffers.sceneDataBuffer = buffer[frame].handle;
		buffers.lightsDataBuffer = lightsBuffer[frame].handle;
		buffers.tilesDataBuffer = tilesBuffer[frame].handle;
		buffers.lightsIndexListBuffer = lightsListBuffer[frame].handle;
		return buffers;
	}

}
