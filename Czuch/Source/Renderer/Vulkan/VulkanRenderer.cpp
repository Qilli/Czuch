#include "czpch.h"
#include "VulkanRenderer.h"
#include"VulkanDevice.h"
#include"VulkanCommandBuffer.h"
#include"VulkanCore.h"
#include"Platform/Windows/WinWindow.h"

#include"Subsystems/Resources/ResourcesManager.h"
#include"Subsystems/Resources/ShaderResource.h"
#include"DescriptorAllocator.h"
#include <glm.hpp>

namespace Czuch
{
	VulkanRenderer::VulkanRenderer(Window* window,ValidationMode validationMode)
	{
		m_RendererValidationMode = validationMode;
		m_AttachedWindow = window;
	}

	Pipeline* pipeline = nullptr;
	Buffer* vertexBufferPos = nullptr;
	Buffer* vertexBufferColor = nullptr;
	Shader* vertexShader = nullptr;
	Buffer* indexBuffer = nullptr;
	Shader* fragmentShader = nullptr;
	bool inited = false;

	VulkanRenderer::~VulkanRenderer()
	{
		m_Device->AwaitDevice();

		m_Device->ReleaseShader(fragmentShader);
		m_Device->ReleaseShader(vertexShader);
		m_Device->ReleaseBuffer(indexBuffer);
		m_Device->ReleaseBuffer(vertexBufferColor);
		m_Device->ReleaseBuffer(vertexBufferPos);
		m_Device->ReleasePipeline(pipeline);
		

		m_Device->ReleaseDescriptorSetLayout(m_SceneData.layout);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_FramesData[a].frameDeletionQueue.Flush();
		}


		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			if (m_SceneData.buffer[a] != nullptr)
			{
				m_Device->ReleaseBuffer(m_SceneData.buffer[a]);
			}
		}
		
		ReleaseSyncObjects();
		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_FramesData[a].descriptorAllocator->CleanUp();
			m_Device->ReleaseDescriptorAllocator(m_FramesData[a].descriptorAllocator);
			m_Device->ReleaseCommandBuffer(m_FramesData[a].commandBuffer);
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
			m_FramesData[a].commandBuffer = (VulkanCommandBuffer*)m_Device->CreateCommandBuffer(true);
			m_FramesData[a].commandBuffer->Init(m_Device);

			m_FramesData[a].descriptorAllocator = m_Device->CreateDescriptorAllocator();
		}

		CreateSyncObjects();
		InitSceneData();
	}

	void VulkanRenderer::DrawFrame()
	{
		if (inited == false)
		{
			inited = true;
			//create objects
			auto resMgr = ResourcesManager::GetPtr();

			auto handle=resMgr->LoadResource<ShaderResource>("F:/Engine/Czuch/Czuch/Data/Shaders/vertShader.vert");
			auto vsRes = resMgr->GetResource<ShaderResource>(handle);

			auto handle2 = resMgr->LoadResource<ShaderResource>("F:/Engine/Czuch/Czuch/Data/Shaders/fragShader.frag");
			auto fsRes = resMgr->GetResource<ShaderResource>(handle2);

			vertexShader = vsRes->GetShaderAsset();
			fragmentShader = fsRes->GetShaderAsset();

			PipelineStateDesc desc;
			desc.vs = vertexShader;
			desc.ps = fragmentShader;
			desc.pt = PrimitiveTopology::TRIANGLELIST;
			desc.rs.cull_mode = CullMode::NONE;
			desc.rs.fill_mode = PolygonMode::SOLID;
			desc.dss.depth_enable = true;
			desc.dss.depth_func = CompFunc::ALWAYS;
			desc.dss.depth_write_mask = DepthWriteMask::ZERO;

			desc.AddLayout(m_SceneData.layout);

			desc.il.AddStream({ .binding = 0,.stride = sizeof(float) * 2,.input_rate = InputClassification::PER_VERTEX_DATA });
			desc.il.AddStream({ .binding = 1,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });

			desc.il.AddAttribute({.location=0,.binding=0,.offset=0,.format=Format::R32G32_FLOAT});
			desc.il.AddAttribute({ .location = 1,.binding = 1,.offset = 0,.format = Format::R32G32B32_FLOAT });

			pipeline=m_Device->CreatePipelineState(&desc, nullptr);

			const std::vector<glm::vec2> positions = {
				{-0.5f, -0.5f},
				{0.5f, -0.5f}, 
				{0.5f, 0.5f},
				{-0.5f, 0.5f},
			};

			const std::vector<glm::vec3> colors = {
				{1.0f, 0.0f, 0.0f},
				{1.0f, 0.0f, 0.0f},
				{1.0f, 0.0f, 0.0f},
				{1.0f, 0.0f, 0.0f},
			};

			const std::vector<U16> indices = { 0, 1, 2, 2, 3, 0 };

			BufferDesc vbDesc;
			vbDesc.elementsCount = 4;
			vbDesc.size = 4 * sizeof(float) * 2;
			vbDesc.stride = 2 * sizeof(float);
			vbDesc.usage = Usage::DEFAULT;
			vbDesc.bind_flags = BindFlag::VERTEX_BUFFER;
			vbDesc.initData = (void*)positions.data();

			vertexBufferPos=m_Device->CreateBuffer(&vbDesc);

			BufferDesc vbDescC;
			vbDescC.elementsCount = 4;
			vbDescC.size = 4 * sizeof(float) * 3;
			vbDescC.stride = 3 * sizeof(float);
			vbDescC.usage = Usage::DEFAULT;
			vbDescC.bind_flags = BindFlag::VERTEX_BUFFER;
			vbDescC.initData = (void*)colors.data();

			vertexBufferColor = m_Device->CreateBuffer(&vbDescC);

			BufferDesc ib;
			ib.elementsCount = 6;
			ib.size = 6 * sizeof(U16);
			ib.stride = sizeof(U16);
			ib.usage = Usage::DEFAULT;
			ib.bind_flags = BindFlag::INDEX_BUFFER;
			ib.initData = (void*)indices.data();

			indexBuffer = m_Device->CreateBuffer(&ib);
		}


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
		vkResetFences(device, 1, &GetCurrentFrame().inFlightFence);
		
		RecordCommandBuffer(imageIndex);

		SubmitCommandBuffer();
		m_Device->Present(imageIndex, GetCurrentFrame().renderFinishedSemaphote);

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
		auto cmdBuffer = GetCurrentFrame().commandBuffer;
		cmdBuffer->Begin();

		cmdBuffer->SetClearColor(0.0f, 1.0f, 0.0f, 1.0f);
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

		cmdBuffer->BindPipeline(pipeline);
		cmdBuffer->BindVertexBuffer(vertexBufferPos,0,0);
		cmdBuffer->BindVertexBuffer(vertexBufferColor, 1, 0);
		cmdBuffer->BindIndexBuffer(indexBuffer,0);
		cmdBuffer->BindDescriptorSet(m_SceneData.descriptor,1,nullptr,0);
		cmdBuffer->DrawIndexed(indexBuffer->desc.elementsCount);

		cmdBuffer->EndCurrentRenderPass();
		cmdBuffer->End();
	}

	void VulkanRenderer::SubmitCommandBuffer()
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkCommandBuffer cmdBuffer = GetCurrentFrame().commandBuffer->GetNativeBuffer();

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
		DescriptorSetLayoutDesc desc{};
		desc.shaderStage = (U32)ShaderStage::PS | (U32)ShaderStage::VS;
		desc.AddBinding(DescriptorType::UNIFORM_BUFFER, 0, 1);
		m_SceneData.layout = m_Device->CreateDescriptorSetLayout(&desc);

		m_SceneData.bufferDesc.createMapped = true;
		m_SceneData.bufferDesc.elementsCount = 1;
		m_SceneData.bufferDesc.bind_flags = BindFlag::UNIFORM_BUFFER;
		m_SceneData.bufferDesc.size = sizeof(SceneData);
		m_SceneData.bufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		m_SceneData.data.ambientColor = vec4(1, 0, 0, 1);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_SceneData.buffer[a] = nullptr;
		}

	}

	void VulkanRenderer::SetSceneData()
	{
		m_SceneData.buffer[m_CurrentFrame] = m_Device->CreateBuffer(&m_SceneData.bufferDesc);
		GetCurrentFrame().frameDeletionQueue.PushFunction([=, this]() {
			if (m_SceneData.buffer[m_CurrentFrame] != nullptr) { m_Device->ReleaseBuffer(m_SceneData.buffer[m_CurrentFrame]); m_SceneData.buffer[m_CurrentFrame] = nullptr; } });
		auto bufferVulkan =Internal_to_Buffer(m_SceneData.buffer[m_CurrentFrame]);

		m_SceneData.data.ambientColor = vec4(1, 1, 0, 1);

		SceneData* data=(SceneData*)bufferVulkan->GetMappedData();
		*data = m_SceneData.data;

		//fill descriptor
		m_SceneData.descriptorSet.Reset();
		m_SceneData.descriptorSet.AddBuffer(m_SceneData.buffer[m_CurrentFrame],0);

		m_SceneData.descriptor =GetCurrentFrame().descriptorAllocator->Allocate(m_SceneData.descriptorSet, m_SceneData.layout);

		DescriptorWriter writer;
		writer.WriteBuffer(0, m_SceneData.buffer[m_CurrentFrame], sizeof(SceneData), 0, DescriptorType::UNIFORM_BUFFER);
		writer.UpdateSet(m_Device->GetNativeDevice(), m_SceneData.descriptor);
	}

	void VulkanRenderer::FrameData::Reset()
	{
		descriptorAllocator->ResetPools();
	}

}
