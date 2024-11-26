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
		m_FillParams.forceMaterialForAll = false;
		m_TextureSource = nullptr;
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
		CZUCH_BE_ASSERT(m_UITextureSource, "This render pass is not a texture source, pointer is null");
		m_TextureSource->Init();
		return m_TextureSource->GetTargetTextureDescriptor();
	}

	void VulkanRenderPassControlBase::SetAsTextureSource()
	{
		if (m_UITextureSource)
		{
			return;
		}
		m_UITextureSource = true;
		m_TextureSource = new UITextureSource(m_Device, m_FrameGraph,&m_FrameGraph->GetNode(m_Node));
	}


	void VulkanRenderPassControlBase::Release()
	{
		if (HANDLE_IS_VALID(m_NativeRenderPassHandle))
		{
			m_Device->Release(m_NativeRenderPassHandle);
		}
		m_Renderer->UnRegisterRenderPassControl(this);

		if (m_TextureSource)
		{
			m_TextureSource->Release();
			delete m_TextureSource;
			m_TextureSource = nullptr;
		}
	}

	void VulkanRenderPassControlBase::Resize(int width, int height)
	{
		m_Width = width;
		m_Height = height;
		if (m_TextureSource)
		{
			m_TextureSource->Release();
		}
	}

}
