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
			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				m_SelectedEntity = {};
				NotifyOnSelectedEntityListeners();
			}

			int flags = ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight;
			//it shows when you click right mouse button on blank space of the window
			if (ImGui::BeginPopupContextWindow(0, flags))
			{
				ImGui::Separator();
				if (ImGui::MenuItem(" Create Empty Entity "))
				{
					m_ActiveScene->CreateEntity("Empty Entity", m_SelectedEntity);
				}
				ImGui::Separator();
				ImGui::EndPopup();
			}
			ImGui::End();
		}
		else
		{
			ImGui::Begin("Scene Hierarchy");
			ImGui::Text("No scene selected");
			ImGui::End();
		}

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
				flags |= ImGuiTreeNodeFlags_Selected;
			}

			auto& header = entity.GetComponent<HeaderComponent>();
			bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)entity, flags, header.GetHeader().c_str());
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				m_SelectedEntity = entity;
				NotifyOnSelectedEntityListeners();
			}

			bool deleteEntity = false;

			if (ImGui::BeginPopupContextItem(header.GetHeader().c_str()))
			{
				ImGui::Separator();
				if (ImGui::MenuItem(" Delete Entity "))
				{
					deleteEntity = true;
					if (m_SelectedEntity == entity)
					{
						m_SelectedEntity = {};
						NotifyOnSelectedEntityListeners();
					}
				}
				ImGui::Separator();
				ImGui::EndPopup();
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

			if (deleteEntity)
			{
				m_ActiveScene->DestroyEntity(entity);
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
