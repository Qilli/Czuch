#include "BaseEditorPanel.h"
#include "EntityInspectorEditorPanel.h"
#include"imgui.h"
#include"Subsystems/Scenes/Components/HeaderComponent.h"
#include"Subsystems/Scenes/Components/TransformComponent.h"

namespace Czuch
{


	void Czuch::EntityInspectorEditorPanel::FillUI()
	{
		if (IsEntityValid(m_SelectedEntity))
		{
			ImGui::Begin("Entity Inspector");

			// Backup the current style colors
			ImVec4 headerColor = ImGui::GetStyle().Colors[ImGuiCol_Header];
			ImVec4 headerHoveredColor = ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered];
			ImVec4 headerActiveColor = ImGui::GetStyle().Colors[ImGuiCol_HeaderActive];

			// Set new colors for the header
			ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0, 0, 0, 0.2f); // Transparent background
			ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImVec4(0, 0, 0, 0.2f); // Transparent background
			ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ImVec4(0, 0, 0, 0.2f); // Transparent background

			m_HeaderDrawer.DrawComponent(m_SelectedEntity);
			m_TransformDrawer.DrawComponent(m_SelectedEntity);

			// Restore the original style colors
			ImGui::GetStyle().Colors[ImGuiCol_Header] = headerColor;
			ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = headerHoveredColor;
			ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = headerActiveColor;

			ImGui::End();
		}
		else
		{
			ImGui::Begin("Entity Inspector");
			ImGui::Text("No entity selected");
			ImGui::End();
		}
	}

	void EntityInspectorEditorPanel::SelectedEntityChanged(Entity entity)
	{
		m_SelectedEntity = entity;
		m_HeaderDrawer.OnSelectionChanged(entity);
	}


	void HeaderDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<HeaderComponent>())
		{
			auto& header = entity.GetComponent<HeaderComponent>();

			int collapseFlags = ImGuiTreeNodeFlags_DefaultOpen| ImGuiTreeNodeFlags_CollapsingHeader;
			if (ImGui::CollapsingHeader(m_HeaderText.c_str(), collapseFlags))
			{
				static char nameBuffer[256];
				std::string currentName = header.GetHeader();
				memset(nameBuffer, 0, sizeof(nameBuffer));
				strcpy_s(nameBuffer,sizeof(nameBuffer), currentName.c_str());
				nameBuffer[currentName.length()] = '\0';  // Ensure null-terminated string

				int flags = ImGuiInputTextFlags_EnterReturnsTrue;
				if(ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer),flags))
				{ 
					header.SetHeader(nameBuffer);
					UpdateTexts(entity);
				}
			}
		}
	}

	void HeaderDrawer::OnSelectionChanged(Entity entity)
	{
		if (entity.IsValid() && entity.HasComponent<HeaderComponent>())
		{
			UpdateTexts(entity);
		}
	}

	void HeaderDrawer::UpdateTexts(Entity entity)
	{
		auto& header = entity.GetComponent<HeaderComponent>();
		strncpy(m_CurrentName, header.GetHeader().c_str(), sizeof(m_CurrentName));
		m_CurrentName[sizeof(m_CurrentName) - 1] = '\0';  // Ensure null-terminated string
		m_HeaderText = "Header Component: " + header.GetHeader();
	}

	void TransformDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<TransformComponent>())
		{
			auto& transform = entity.GetComponent<TransformComponent>();

			int collapseFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_CollapsingHeader;
			if (ImGui::CollapsingHeader("Transform Component", collapseFlags))
			{
				glm::vec3 position = transform.GetLocalPosition();

				if (ImGui::DragFloat3("Position", &position.x, 0.1f, 0.0f, 0.0f, "%.3f"))
				{
					transform.SetLocalPosition(position);
				}

			}
		}
	}

	void TransformDrawer::OnSelectionChanged(Entity entity)
	{
	}
}