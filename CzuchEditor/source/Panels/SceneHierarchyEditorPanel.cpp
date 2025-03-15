#include "SceneHierarchyEditorPanel.h"
#include"imgui.h"
#include"Subsystems/Scenes/Components/HeaderComponent.h"
#include"Subsystems/Scenes/Components/TransformComponent.h"
#include"../Commands/EditorCommandsControl.h"
#include"../Commands/CommandTypes/CreateNewEntityCommand.h"
#include"../Commands/CommandTypes/RemoveEntityCommand.h"
#include"../Commands/CommandTypes/CreateDefaultMeshesCommands.h"
#include"../Commands/CommandTypes/ReparentEntityCommand.h"

namespace Czuch
{
	SceneHierarchyEditorPanel::SceneHierarchyEditorPanel(Scene* activeScene)
	{
		SetActiveScene(activeScene);
	}

	void SceneHierarchyEditorPanel::SetActiveScene(Scene* activeScene)
	{
		if (m_ActiveScene != activeScene)
		{
			m_SelectedEntity = {};
			NotifyOnSelectedEntityListeners();
		}
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
					EditorCommandsControl::Get().ExecuteCommand(new CreateNewEntityCommand(m_ActiveScene, m_SelectedEntity));
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

			if (transformComponent.HasAnyChild())
			{
				flags |= (ImGuiTreeNodeFlags_OpenOnArrow);
			}
			else
			{
				flags |= (ImGuiTreeNodeFlags_Leaf);
			}

			bool popStyle = false;
			if (m_SelectedEntity == entity)
			{
				flags |= ImGuiTreeNodeFlags_Selected;
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.85f, 0.1f, 1.0f)); // green color
				popStyle = true;
			}
			auto& guid = entity.GetComponent<GUIDComponent>();

			auto& header = entity.GetComponent<HeaderComponent>();
			bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)entity.GetID(), flags, header.GetHeader().c_str());

			// Start drag-and-drop source
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				ImGui::SetDragDropPayload("TREE_NODE", &guid, sizeof(GUIDComponent));
				ImGui::EndDragDropSource();
			}

			// Handle drag-and-drop target
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TREE_NODE"))
				{
					GUIDComponent* droppedData = (GUIDComponent*)payload->Data;
					if (droppedData->GetGUID() != guid.GetGUID())
					{
						auto droppedEntity = m_ActiveScene->GetEntityObjectWithGUID(droppedData->GetGUID());
						EditorCommandsControl::Get().ExecuteCommand(NEW(ReparentEntityCommand(m_ActiveScene,droppedEntity, entity)));
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (popStyle)
			{
				ImGui::PopStyleColor();
			}

			if ((ImGui::IsItemActivated() || ImGui::IsItemClicked()|| ImGui::IsItemClicked(1)) && !ImGui::IsItemToggledOpen())
			{
				m_SelectedEntity = entity;
				NotifyOnSelectedEntityListeners();
			}


			if (m_SelectedEntity == entity)
			{

				if (ImGui::BeginPopupContextItem(guid.GetGUID().ToString().c_str()))
				{
					ImGui::Separator();

					if (ImGui::MenuItem(" Create Child Entity "))
					{
						auto command = new CreateNewEntityCommand(m_ActiveScene, m_SelectedEntity);
						EditorCommandsControl::Get().ExecuteCommand(command);

						m_SelectedEntity = command->GetCreatedEntity();
						NotifyOnSelectedEntityListeners();
					}

					if (ImGui::MenuItem(" Create Plane Mesh"))
					{
						auto command = NEW(CreateNewEntityWithPlaneMeshCommand(m_ActiveScene, m_SelectedEntity));
						EditorCommandsControl::Get().ExecuteCommand(command);

						m_SelectedEntity = command->GetCreatedEntity();
						NotifyOnSelectedEntityListeners();
					}

					if (ImGui::MenuItem(" Create Cube Mesh"))
					{
						auto command = NEW(CreateNewEntityWithCubeMeshCommand(m_ActiveScene, m_SelectedEntity));
						EditorCommandsControl::Get().ExecuteCommand(command);

						m_SelectedEntity = command->GetCreatedEntity();
						NotifyOnSelectedEntityListeners();
					}

					if (ImGui::MenuItem(" Delete Entity "))
					{
						EditorCommandsControl::Get().ExecuteCommand(new RemoveNewEntityCommand(m_ActiveScene, m_SelectedEntity));
						if (m_SelectedEntity == entity)
						{
							m_SelectedEntity = {};
							NotifyOnSelectedEntityListeners();
						}
					}
					ImGui::Separator();
					ImGui::EndPopup();
				}

			}


			if (node_open)
			{
				//draw children
				auto transformComponent = entity.GetComponent<TransformComponent>();
				for (auto child : transformComponent.GetChildren())
				{
					auto& header = child.GetComponent<HeaderComponent>();
					if (child.IsDestroyed() || !header.IsVisibleInEditorHierarchy())
					{
						continue;
					}
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
