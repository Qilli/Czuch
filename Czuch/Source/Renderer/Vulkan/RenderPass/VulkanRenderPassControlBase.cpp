#include "czpch.h"
#include "VulkanRenderPassControlBase.h"
#include "../VulkanRenderer.h"
#include "../VulkanDevice.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
namespace Czuch
{
	VulkanRenderPassControlBase::VulkanRenderPassControlBase(VulkanDevice* device, VulkanRenderer* renderer, Camera* cam, U32 width, U32 height, RenderPassType type, bool handleWindowResize) :RenderPassControl(cam, width, height, type, handleWindowResize, false), m_Renderer(renderer), m_Device(device)
	{
		m_FillParams.renderPassType = type;
	}
	VulkanRenderPassControlBase::~VulkanRenderPassControlBase()
	{
		Release();
	}
	void VulkanRenderPassControlBase::PreDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		m_Renderer->OnPreRenderUpdateContexts(m_Camera, m_Width, m_Height, &m_FillParams);
	}
	void VulkanRenderPassControlBase::PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		m_Renderer->OnPostRenderUpdateContexts(&m_FillParams);
	}
	void VulkanRenderPassControlBase::Execute(CommandBuffer* cmdBuffer)
	{
		m_Renderer->DrawScene((VulkanCommandBuffer*)cmdBuffer);
	}
	void* VulkanRenderPassControlBase::GetRenderPassResult()
	{
		return m_TextureSource.GetTargetTextureDescriptor();
	}

	void VulkanRenderPassControlBase::SetAsTextureSource()
	{
		if (m_UITextureSource)
		{
			return;
		}
		m_UITextureSource = true;
		m_TextureSource.Init();

	}


	void VulkanRenderPassControlBase::Release()
	{
		if (HANDLE_IS_VALID(m_NativeRenderPassHandle))
		{
			m_Device->Release(m_NativeRenderPassHandle);
		}
		m_Renderer->UnRegisterRenderPassControl(this);
	}

}
