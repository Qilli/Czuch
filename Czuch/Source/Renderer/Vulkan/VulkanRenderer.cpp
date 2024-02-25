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
#include"DescriptorAllocator.h"
#include"Core/Math.h"

namespace Czuch
{
	VulkanRenderer::VulkanRenderer(Window* window,ValidationMode validationMode)
	{
		m_RendererValidationMode = validationMode;
		m_AttachedWindow = window;
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
		if (inited == false)
		{
			inited = true;
			//create objects
			auto resMgr = AssetsManager::GetPtr();

			auto handle = resMgr->LoadAsset<ShaderAsset, LoadSettingsDefault>("F:/Engine/Czuch/Czuch/Data/Shaders/vertShader.vert", {});
			auto vsRes = resMgr->GetAsset<ShaderAsset>(handle);

			auto handle2 = resMgr->LoadAsset<ShaderAsset>("F:/Engine/Czuch/Czuch/Data/Shaders/fragShader.frag");
			auto fsRes = resMgr->GetAsset<ShaderAsset>(handle2);

			auto texHandle = resMgr->LoadAsset<TextureAsset, TextureLoadSettings>("F:/Engine/Czuch/Czuch/Data/Textures/texture.jpg", {.type = TextureDesc::Type::TEXTURE_2D});

			std::vector<U8> colors1;
			colors1.reserve(128 * 128 * 4);

			for (size_t i = 0; i < 128; i++)
			{
				for (size_t j = 0; j < 128; j++)
				{
					colors1.push_back(255);
					colors1.push_back(255);
					colors1.push_back(255);
					colors1.push_back(255);
				}
			}

			TextureCreateSettings createTexSet;
			createTexSet.channels = 4;
			createTexSet.height = 128;
			createTexSet.width = 128;
			createTexSet.colors = colors1;

			auto texHandle2 = resMgr->CreateAsset<TextureAsset, TextureCreateSettings>("White",createTexSet);


			PipelineStateDesc desc;
			desc.vs = handle;
			desc.ps = handle2;
			desc.pt = PrimitiveTopology::TRIANGLELIST;
			desc.rs.cull_mode = CullMode::NONE;
			desc.rs.fill_mode = PolygonMode::SOLID;
			desc.dss.depth_enable = true;
			desc.dss.depth_func = CompFunc::LESS_EQUAL;
			desc.dss.depth_write_mask = DepthWriteMask::ZERO;
			desc.dss.stencil_enable = false;

			desc.il.AddStream({ .binding = 0,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });
			desc.il.AddStream({ .binding = 1,.stride = sizeof(float) * 3,.input_rate = InputClassification::PER_VERTEX_DATA });
			desc.il.AddStream({ .binding = 2,.stride = sizeof(float) * 2,.input_rate = InputClassification::PER_VERTEX_DATA });

			desc.il.AddAttribute({.location=0,.binding=0,.offset=0,.format=Format::R32G32B32_FLOAT});
			desc.il.AddAttribute({ .location = 1,.binding = 1,.offset = 0,.format = Format::R32G32B32_FLOAT });
			desc.il.AddAttribute({ .location = 2,.binding = 2,.offset = 0,.format = Format::R32G32_FLOAT });


			MaterialDesc matDesc;
			matDesc.pipelineDesc = std::move(desc);
			matDesc.materialName = "DefaultMaterial";


			DescriptorSetLayoutDesc desc_1{};
			desc_1.shaderStage = (U32)ShaderStage::PS | (U32)ShaderStage::VS;
			desc_1.AddBinding("SceneData", DescriptorType::UNIFORM_BUFFER, 0, 1,sizeof(SceneData));

			DescriptorSetLayoutDesc desc_tex{};
			desc_tex.shaderStage = (U32)ShaderStage::PS;
			desc_tex.AddBinding("MainTexture", DescriptorType::SAMPLER, 0, 1,0);

			DescriptorSetDesc setDesc_1;
			DescriptorSetDesc setDesc_Tex;

			setDesc_1.AddBuffer(m_SceneData.buffer[0], 0);
			setDesc_Tex.AddSampler(m_SceneData.tex, 0);

			matDesc.AddLayoutWithDescriptor(desc_1, setDesc_1);
			matDesc.AddLayoutWithDescriptor(desc_tex, setDesc_Tex);

			MaterialCreateSettings createSettings;
			createSettings.desc = std::move(matDesc);

			m_SceneData.materialHandle = resMgr->CreateAsset<MaterialAsset, MaterialCreateSettings>(createSettings.desc.materialName, createSettings);


			const std::vector<glm::vec3> positions = {
				{-0.5f, -0.5f,0.0f},
				{0.5f, -0.5f,0.0f}, 
				{0.5f, 0.5f,0.0f},
				{-0.5f, 0.5f,0.0f},
			};

			const std::vector<glm::vec3> colors = {
				{1.0f, 0.0f, 0.0f},
				{1.0f, 0.0f, 0.0f},
				{1.0f, 0.0f, 0.0f},
				{1.0f, 0.0f, 0.0f},
			};


			const std::vector<glm::vec2> uvs = {
				{0.0f, 0.0f},
				{1.0f, 0.0f},
				{1.0f, 1.0f},
				{0.0f, 1.0f},
			};

			const std::vector<U16> indices = { 0, 1, 2, 2, 3, 0 };

			BufferDesc vbDesc;
			vbDesc.elementsCount = 4;
			vbDesc.size = 4 * sizeof(float) * 3;
			vbDesc.stride = 3 * sizeof(float);
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


			BufferDesc vbDescUV;
			vbDescUV.elementsCount = 4;
			vbDescUV.size = 4 * sizeof(float) * 2;
			vbDescUV.stride = 2 * sizeof(float);
			vbDescUV.usage = Usage::DEFAULT;
			vbDescUV.bind_flags = BindFlag::VERTEX_BUFFER;
			vbDescUV.initData = (void*)uvs.data();

			vertexBufferUV = m_Device->CreateBuffer(&vbDescUV);


			BufferDesc ib;
			ib.elementsCount = 6;
			ib.size = 6 * sizeof(U16);
			ib.stride = sizeof(U16);
			ib.usage = Usage::DEFAULT;
			ib.bind_flags = BindFlag::INDEX_BUFFER;
			ib.initData = (void*)indices.data();

			indexBuffer = m_Device->CreateBuffer(&ib);

			auto texRes = resMgr->GetAsset<TextureAsset>(texHandle2);
			m_SceneData.tex = texRes->GetTextureAssetHandle();
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

		cmdBuffer->BindPipeline(m_SceneData.pipeline);
		cmdBuffer->BindVertexBuffer(vertexBufferPos,0,0);
		cmdBuffer->BindVertexBuffer(vertexBufferColor, 1, 0);
		cmdBuffer->BindVertexBuffer(vertexBufferUV, 2, 0);
		cmdBuffer->BindIndexBuffer(indexBuffer,0);
		cmdBuffer->BindDescriptorSet(m_SceneData.descriptor,0,1,nullptr,0);
		cmdBuffer->BindDescriptorSet(m_SceneData.descriptorTex,1, 1, nullptr, 0);
		cmdBuffer->DrawIndexed(m_Device->AccessBuffer(indexBuffer)->desc.elementsCount);

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


		auto materialAsset= AssetsManager::GetPtr()->GetAsset<MaterialAsset>(m_SceneData.materialHandle);
		Material* m = m_Device->AccessMaterial(materialAsset->GetMaterialAssetHandle());


		m->desc.descriptorsDesc[0].Reset();
		m->desc.descriptorsDesc[0].AddBuffer(m_SceneData.buffer[m_CurrentFrame], 0);


		auto pipelineObj = m_Device->AccessPipeline(m_SceneData.pipeline);

		auto layout_0=pipelineObj->layouts[0];
		auto layout_1 = pipelineObj->layouts[1];

		m_SceneData.descriptor =GetCurrentFrame().descriptorAllocator->Allocate(m_SceneData.descriptorSet, m_Device->AccessDescriptorSetLayout(layout_0));
		m_SceneData.descriptorTex = GetCurrentFrame().descriptorAllocator->Allocate(m_SceneData.descriptorSetTex, m_Device->AccessDescriptorSetLayout(layout_1));

		DescriptorWriter writer;
		writer.WriteBuffer(0, m_Device->AccessBuffer(m_SceneData.buffer[m_CurrentFrame]), sizeof(SceneData), 0, DescriptorType::UNIFORM_BUFFER);
		writer.UpdateSet(m_Device, m_SceneData.descriptor);

		writer.Clear();
		writer.WriteTexture(0, m_Device->AccessTexture(m_SceneData.tex), DescriptorType::SAMPLER);
		writer.UpdateSet(m_Device, m_SceneData.descriptorTex);
	}

	void VulkanRenderer::FrameData::Reset()
	{
		descriptorAllocator->ResetPools();
	}

}
