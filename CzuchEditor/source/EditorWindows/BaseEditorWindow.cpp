#include "BaseEditorWindow.h"
#include"imgui.h"
#include "Czuch.h"
namespace Czuch
{
	BaseEditorWindow::BaseEditorWindow()
		:m_WindowName("Window"), m_IsVisible(false), m_IsInited(false)
	{
	}
	void BaseEditorWindow::DrawWindow(EngineRoot* root)
	{
		if (m_IsVisible)
		{
			if (m_IsInited == false)
			{
				Init(root);
				m_IsInited = true;
			}

			if (ImGui::Begin(m_WindowName, &m_IsVisible, ImGuiWindowFlags_AlwaysAutoResize))
			{
				FillUI();
			}
			ImGui::End();
		}
	}
}
