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
		m_GeneralRenderContext = nullptr;
		m_DebugRenderContext = nullptr;
	}
	VulkanRenderPassControlBase::~VulkanRenderPassControlBase()
	{
		Release();
		if (m_DebugRenderContext != nullptr)
		{
			delete m_DebugRenderContext;
		}
		if (m_GeneralRenderContext != nullptr)
		{
			delete m_GeneralRenderContext;
		}
	}
	void VulkanRenderPassControlBase::PreDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		m_Renderer->OnPreRenderUpdateContexts(m_Camera, m_Width, m_Height, &m_FillParams, m_GeneralRenderContext);
	}
	void VulkanRenderPassControlBase::PostDraw(CommandBuffer* cmdBuffer, Renderer* renderer)
	{
		m_Renderer->OnPostRenderUpdateContexts(&m_FillParams, m_GeneralRenderContext);
	}

	void VulkanRenderPassControlBase::BindInputTextures(GraphicsDevice* device, FrameGraphNode* node)
	{
		if (m_MaterialsForInputBinding.m_Materials.empty())
		{
			return;
		}
		for (auto resourceHandle : node->inputs)
		{
			auto& resource = m_FrameGraph->GetResource(resourceHandle);
			if (resource.type == FrameGraphResourceType::Texture)
			{
				auto& res = m_FrameGraph->GetResource(resource.output_target);
				TextureHandle texture = res.info.texture.texture;
				m_MaterialsForInputBinding.BindTextureForMaterials(device, resource.nameID, texture);
			}
		}
	}

	void VulkanRenderPassControlBase::Execute(CommandBuffer* cmdBuffer)
	{
		m_Renderer->DrawScene((VulkanCommandBuffer*)cmdBuffer,m_GeneralRenderContext);
	}
	void* VulkanRenderPassControlBase::GetRenderPassResult()
	{
		CZUCH_BE_ASSERT(m_UITextureSource, "This render pass is not a texture source, pointer is null");
		m_TextureSource->Init();

		return m_TextureSource->GetTargetTextureDescriptor();
	}

	void VulkanRenderPassControlBase::TransitionResultsToShaderReadOnly(CommandBuffer* cmd)
	{
		if (m_TextureSource)
		{
			m_TextureSource->TransitionToShaderReadOnly(cmd);
		}
	}

	void VulkanRenderPassControlBase::SetAsTextureSource()
	{
		if (m_Renderer->GetRenderSettings().engineMode == EngineMode::Runtime)
		{
			return;
		}

		if (m_UITextureSource)
		{
			return;
		}
		m_UITextureSource = true;
		m_TextureSource = new UITextureSource(m_Device, m_FrameGraph, &m_FrameGraph->GetNode(m_Node));
	}


	void VulkanRenderPassControlBase::Release()
	{
		if (HANDLE_IS_VALID(m_NativeRenderPassHandle))
		{
			m_Device->Release(m_NativeRenderPassHandle);
		}
		m_Renderer->UnRegisterRenderPassControl(this);

		if (m_TextureSource != nullptr)
		{
			m_TextureSource->Release();
			delete m_TextureSource;
			m_TextureSource = nullptr;
		}

		if (m_DebugRenderContext != nullptr)
		{
			m_DebugRenderContext->ClearRenderList();
		}

		if (m_GeneralRenderContext != nullptr)
		{
			m_GeneralRenderContext->ClearRenderList();
		}
	}

	void VulkanRenderPassControlBase::ReleaseDependencies()
	{
		if (m_TextureSource != nullptr)
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

	void VulkanRenderPassControlBase::Init()
	{
		CreateRenderContexts();
	}

	void VulkanRenderPassControlBase::CreateRenderContexts()
	{
		RenderContextCreateInfo renderContextCreateInfo{};
		renderContextCreateInfo.autoClearAfterRender = true;
		renderContextCreateInfo.renderType = RenderType::General;
		renderContextCreateInfo.renderLayer = RenderLayer::LAYER_0;
		renderContextCreateInfo.sortingOrder = 0;
		m_GeneralRenderContext = new DefaultRenderContext(renderContextCreateInfo);

		renderContextCreateInfo.renderType = RenderType::Debug;
		renderContextCreateInfo.renderLayer = RenderLayer::LAYER_0;
		m_DebugRenderContext =new DefaultRenderContext(renderContextCreateInfo);
	}

}
