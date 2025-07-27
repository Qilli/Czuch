#include "czpch.h"
#include "RenderPassControl.h"
#include "FrameGraph/FrameGraph.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include "Renderer.h"
#include"GraphicsDevice.h"

namespace Czuch
{
	RenderPassControl::RenderPassControl(Camera* cam, int width, int height, RenderPassType type, bool handleWindowResize, bool uiTextureSource) : m_Width(width), m_Height(height), Type(type), m_HandleWindowResize(handleWindowResize), m_Camera(cam), m_Priority(10), m_UITextureSource(uiTextureSource)
	{
		m_LastWidth = m_Width;
		m_LastHeight = m_Height;
		m_LastCamera = m_Camera;
	}


	void RenderPassControl::PreDraw(CommandBuffer* cmd,Renderer* renderer)
	{

	}

	void RenderPassControl::PostDraw(CommandBuffer* cmd, Renderer* renderer)
	{
		
	}

	void RenderPassControl::BeforeFrameGraphExecute(CommandBuffer* cmd, Renderer* renderer)
	{
		if (IsDifferentAspect(m_LastWidth, m_LastHeight) || IsDifferentCamera(m_LastCamera))
		{
			m_LastWidth = GetWidth();
			m_LastHeight = GetHeight();
			m_LastCamera = GetCamera();
			renderer->OnPreRenderUpdateContexts(m_Camera, m_LastWidth, m_LastHeight, &m_FillParams);
		}
		else
		{
			renderer->OnPreRenderUpdateContexts(m_Camera, m_Width, m_Height, &m_FillParams);
		}
	}

	void RenderPassControl::AfterFrameGraphExecute(CommandBuffer* cmd, Renderer* renderer)
	{
		renderer->OnPostRenderUpdateContexts(m_Camera, &m_FillParams);
	}

	void RenderPassControl::Resize(int width, int height)
	{
		m_Width = width; m_Height = height;
	}

	void RenderPassControl::AddMaterialForInputBinding(MaterialInstanceHandle material)
	{
		m_MaterialsForInputBinding.AddMaterial(material);
	}

	void RenderPassControl::ForceSingleMaterialForAll(MaterialInstanceHandle material, bool ignoreTransparent)
	{
		m_FillParams.forcedMaterial = material;
		m_FillParams.forceMaterialForAll = true;
		m_FillParams.ignoreTransparent = ignoreTransparent;
	}

	void RenderPassControl::ClearForceSingleMaterialForAll()
	{
		m_FillParams.forcedMaterial = INVALID_HANDLE(MaterialInstanceHandle);
		m_FillParams.forceMaterialForAll = false;
		m_FillParams.ignoreTransparent = false;
	}


	void MaterialsForInputBinding::BindTextureForMaterials(GraphicsDevice* device, StringID& texName, TextureHandle texture)
	{
		for (auto& mat : m_Materials)
		{
			MaterialInstance* matInstance = device->AccessMaterialInstance(mat);
			matInstance->SetSampler(texName, texture);
		}
	}

}
