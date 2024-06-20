#include "SceneHierarchyEditorPanel.h"
#include"imgui.h"
#include"Subsystems/Scenes/Components/HeaderComponent.h"
#include"Subsystems/Scenes/Components/TransformComponent.h"

namespace Czuch
{
	SceneHierarchyEditorPanel::SceneHierarchyEditorPanel(Scene* activeScene)
	{
		SetActiveScene(activeScene);
	}

	void SceneHierarchyEditorPanel::SetActiveScene(Scene* activeScene)
	{
		m_ActiveScene = activeScene;
	}

	void SceneHierarchyEditorPanel::FillUI()
	{
		if (m_ActiveScene)
		{
			ImGui::Begin(m_ActiveScene->GetSceneName().c_str());;
			ImGui::Separator();

			DrawEntityNode(m_ActiveScene->GetRootEntity());
		}

		ImGui::End();
	}

	void SceneHierarchyEditorPanel::AddOnSelectedEntityListener(BaseEditorPanel* listener)
	{
		m_OnSelectedEntityListeners.push_back(listener);
	}

	void SceneHierarchyEditorPanel::DrawEntityNode(Entity entity)
	{
		if (entity.HasComponent<HeaderComponent>())
		{
			auto transformComponent = entity.GetComponent<TransformComponent>();

			int flags = ImGuiTreeNodeFlags_SpanAvailWidth;

			if (transformComponent.GetChildren().size() > 0)
			{
				flags |= (ImGuiTreeNodeFlags_OpenOnArrow);
			}	
			else
			{
				flags |= (ImGuiTreeNodeFlags_Leaf);
			}

			if (m_SelectedEntity == entity)
			{
				flags |=  ImGuiTreeNodeFlags_Selected;
			}

			auto& header = entity.GetComponent<HeaderComponent>();
			bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)entity,flags, header.GetHeader().c_str());
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				m_SelectedEntity = entity;
				NotifyOnSelectedEntityListeners();
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				m_SelectedEntity = {};
				NotifyOnSelectedEntityListeners();
			}

			if (node_open)
			{
				//draw children
				auto transformComponent = entity.GetComponent<TransformComponent>();
				for (auto child : transformComponent.GetChildren())
				{
					DrawEntityNode(child);
				}
				ImGui::TreePop();
			}
		}
	}

	void SceneHierarchyEditorPanel::NotifyOnSelectedEntityListeners()
	{
		for (auto listener : m_OnSelectedEntityListeners)
		{
			listener->SelectedEntityChanged(m_SelectedEntity);
		}
	}
}
