#include"czpch.h"
#include "VulkanDepthLinearPrepassRenderPass.h"
#include "../VulkanRenderer.h"
#include"Subsystems/Assets/BuildInAssets.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include "../VulkanDevice.h"


namespace Czuch
{
	VulkanDepthLinearPrepassRenderPass::VulkanDepthLinearPrepassRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, RenderPassType renderPass, MaterialInstanceHandle* materialInstance, bool handleWindowResize) :VulkanRenderPassControlBase(device,renderer,nullptr,width,height,renderPass,handleWindowResize)
	{
		m_MaterialInstancePtr = materialInstance;
	}

	void VulkanDepthLinearPrepassRenderPass::Execute(CommandBuffer* cmd)
	{
		m_Renderer->DrawFullScreenQuad((VulkanCommandBuffer*)cmd, *m_MaterialInstancePtr);
	}
	void VulkanDepthLinearPrepassRenderPass::PreDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		VulkanRenderPassControlBase::PreDraw(cmdBuffer, renderer);
		if (!m_MaterialsAdded)
		{
			m_MaterialsAdded = true;
			AddMaterialForInputBinding(*m_MaterialInstancePtr);

			//create new uniform buffer
			BufferDesc desc{};
			desc.size = sizeof(CameraPlanesData);
			desc.usage = Usage::MEMORY_USAGE_GPU_ONLY;
			desc.bind_flags = BindFlag::UNIFORM_BUFFER;
			desc.persistentMapped = false;
			desc.initData = &m_PlanesData;
			m_PlanesBuffer=m_Device->CreateBuffer(&desc);
			m_PlanesBufferID = StringID::MakeStringID("CameraPlanesData");
		}
		MaterialInstance* mat = m_Device->AccessMaterialInstance(*m_MaterialInstancePtr);
		mat->SetUniformBuffer(m_PlanesBufferID, m_PlanesBuffer);
	}
	void VulkanDepthLinearPrepassRenderPass::Init()
	{
		VulkanRenderPassControlBase::Init();
		m_PlanesData.nearPlane = 1.0f;
		m_PlanesData.farPlane = 50.0f;
	}
	void VulkanDepthLinearPrepassRenderPass::Release()
	{
		VulkanRenderPassControlBase::Release();
		if (HANDLE_IS_VALID(m_PlanesBuffer))
		{
			m_Device->Release(m_PlanesBuffer);
		}
	}
}