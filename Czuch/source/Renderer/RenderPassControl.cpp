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
		if (IsDifferentAspect(m_LastWidth, m_LastHeight) || IsDifferentCamera(m_LastCamera))
		{
			renderer->OnPostRenderUpdateContexts(&m_FillParams,&GetMainRenderContext());
			m_LastWidth = GetWidth();
			m_LastHeight = GetHeight();
			m_LastCamera = GetCamera();
			renderer->OnPreRenderUpdateContexts(m_Camera, m_LastWidth, m_LastHeight,&m_FillParams,&GetMainRenderContext());
		}
		else
		{
			renderer->OnPreRenderUpdateContexts(m_Camera, m_Width, m_Height,&m_FillParams,&GetMainRenderContext());
		}
	}

	void RenderPassControl::Resize(int width, int height)
	{
		m_Width = width; m_Height = height;
	}

	void RenderPassControl::AddMaterialForInputBinding(MaterialInstanceHandle material)
	{
		m_MaterialsForInputBinding.AddMaterial(material);
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
