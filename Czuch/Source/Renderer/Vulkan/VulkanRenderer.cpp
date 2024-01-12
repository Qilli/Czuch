#include "czpch.h"
#include "VulkanRenderer.h"
#include"VulkanDevice.h"
#include"VulkanCommandBuffer.h"
#include"VulkanCore.h"
#include"Platform/Windows/WinWindow.h"

#include"Subsystems/Resources/ResourcesManager.h"
#include"Subsystems/Resources/ShaderResource.h"
#include <glm.hpp>

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

	Pipeline* pipeline = nullptr;
	Buffer* vertexBufferPos = nullptr;
	Buffer* vertexBufferColor = nullptr;
	Buffer* indexBuffer = nullptr;
	Shader* vertexShader = nullptr;
	Shader* fragmentShader = nullptr;
	bool inited = false;

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

		m_CmdBuffers[m_CurrentFrame]->SetClearColor(0.0f, 1.0f, 0.0f, 1.0f);
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

		m_CmdBuffers[m_CurrentFrame]->BindPipeline(pipeline);
		m_CmdBuffers[m_CurrentFrame]->BindVertexBuffer(vertexBufferPos,0,0);
		m_CmdBuffers[m_CurrentFrame]->BindVertexBuffer(vertexBufferColor, 1, 0);
		m_CmdBuffers[m_CurrentFrame]->BindIndexBuffer(indexBuffer,0);
		m_CmdBuffers[m_CurrentFrame]->DrawIndexed(indexBuffer->desc.elementsCount);

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
