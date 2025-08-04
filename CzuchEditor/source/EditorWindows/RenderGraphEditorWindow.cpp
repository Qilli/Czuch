#include "RenderGraphEditorWindow.h"
#include"imgui.h"
#include "Czuch.h"

namespace Czuch
{
	FrameGraph* m_FrameGraph;
	const ImVec2 m_Size = ImVec2(256, 256);

	RenderGraphEditorWindow::RenderGraphEditorWindow(const char* name) :BaseEditorWindow()
	{
		SetWindowName(name);
	}

	RenderGraphEditorWindow::~RenderGraphEditorWindow()
	{
	}

	void RenderGraphEditorWindow::FillUI()
	{
		ImGui::Text("Render Graph results preview");
		ImGui::Separator();
		ImGui::Text("Nodes");
		ImGui::Separator();
		U32 nodesCount=m_FrameGraph->GetNodesCount();
		for (U32 i = 0; i < nodesCount; i++)
		{
			auto nodeResult = m_FrameGraph->GetRenderPassResultAt(i);
			if (nodeResult == nullptr)
			{
				continue;
			}
			ImGui::Text("Node name:%s", m_FrameGraph->GetNodeNameAt(i).c_str());
			ImGui::Image((ImTextureID)nodeResult, m_Size);
			ImGui::Separator();
		}
	}


	void RenderGraphEditorWindow::Init(EngineRoot* root)
	{
		m_FrameGraph = root->GetScenesManager().GetActiveScene()->GetFrameGraphControl(0)->GetFrameGraph();
	}
}