#include "AssetsInfoEditorWindow.h"
#include"imgui.h"
#include "Czuch.h"

namespace Czuch
{

	AssetsInfoEditorWindow::AssetsInfoEditorWindow(const char* name) :BaseEditorWindow()
	{
		SetWindowName(name);
	}

	AssetsInfoEditorWindow::~AssetsInfoEditorWindow()
	{
	}

	void DrawSingleMgrType(AssetManager* mgr)
	{
		for (auto it = mgr->GetAssetIterator(); it != mgr->GetAssetEnd(); it++)
		{
			ImGui::Text((*it).second->GetAssetLoadInfo().c_str());
		}
	}

	void AssetsInfoEditorWindow::FillUI()
	{
		ImGui::Text("List of all assets in assets manager");
		ImGui::Separator();
		ImGui::Text("Assets:");

		auto mgr = AssetsManager::GetPtr();

		for (auto it=mgr->GetAssetManagerIterator();it!=mgr->GetAssetManagerEnd();it++)
		{
			const auto& singleMgr = (*it).second;
			DrawSingleMgrType(singleMgr);
		}
		
	}


	void AssetsInfoEditorWindow::Init(EngineRoot* root)
	{
		
	}
}