#include "czpch.h"
#include "RenderPassControl.h"
#include "FrameGraph/FrameGraph.h"
#include "Renderer.h"

namespace Czuch
{
	void RenderPassControl::PreDraw(CommandBuffer* cmd,Renderer* renderer)
	{
		if (IsDifferentAspect(m_LastWidth, m_LastHeight) || IsDifferentCamera(m_LastCamera))
		{
			renderer->OnPostRenderUpdateContexts();
			m_LastWidth = GetWidth();
			m_LastHeight = GetHeight();
			m_LastCamera = GetCamera();
			renderer->OnPreRenderUpdateContexts(m_Camera, m_LastWidth, m_LastHeight);
		}
		else
		{
			renderer->OnPreRenderUpdateContexts(m_Camera, m_Width, m_Height);
		}
	}

	void RenderPassControl::Resize(int width, int height)
	{
		m_Width = width; m_Height = height;
	}

}
