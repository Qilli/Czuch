#include"czpch.h"
#include "VulkanDepthLinearPrepassRenderPass.h"
#include "../VulkanRenderer.h"
#include"Subsystems/Assets/BuildInAssets.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include "../VulkanDevice.h"


namespace Czuch
{
	VulkanDepthLinearPrepassRenderPass::VulkanDepthLinearPrepassRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize) :VulkanRenderPassControlBase(device,renderer,nullptr,width,height,RenderPassType::DepthLinearPrePass,handleWindowResize)
	{
		
	}

	void VulkanDepthLinearPrepassRenderPass::Execute(CommandBuffer* cmd)
	{
		m_Renderer->DrawFullScreenQuad((VulkanCommandBuffer*)cmd, DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_INSTANCE);
	}
	void VulkanDepthLinearPrepassRenderPass::PreDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		if (!m_MaterialsAdded)
		{
			m_MaterialsAdded = true;
			AddMaterialForInputBinding(DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_INSTANCE);

			//create new uniform buffer
			BufferDesc desc{};
			desc.size = sizeof(CameraPlanesData);
			desc.usage = Usage::MEMORY_USAGE_GPU_ONLY;
			desc.bind_flags = BindFlag::UNIFORM_BUFFER;
			desc.createMapped = false;
			desc.initData = &m_PlanesData;
			m_PlanesBuffer=m_Device->CreateBuffer(&desc);
			m_PlanesBufferID = StringID::MakeStringID("CameraPlanesData");
		}
		MaterialInstance* mat = m_Device->AccessMaterialInstance(DefaultAssets::DEPTH_LINEAR_PREPASS_MATERIAL_INSTANCE);
		mat->SetUniformBuffer(m_PlanesBufferID, m_PlanesBuffer);
	}
	void VulkanDepthLinearPrepassRenderPass::Init()
	{
		VulkanRenderPassControlBase::Init();
		m_PlanesData.nearPlane = 0.1f;
		m_PlanesData.farPlane = 1000.0f;
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