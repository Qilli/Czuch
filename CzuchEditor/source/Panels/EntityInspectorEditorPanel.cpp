#include "BaseEditorPanel.h"
#include "EntityInspectorEditorPanel.h"
#include"imgui.h"
#include"imgui_internal.h"
#include"Subsystems/Scenes/Components/HeaderComponent.h"
#include"Subsystems/Scenes/Components/TransformComponent.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"

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
			m_CameraDrawer.DrawComponent(m_SelectedEntity);
			ImGui::Separator();
			AddComponentMenu();

			// Restore the original style colors
			ImGui::GetStyle().Colors[ImGuiCol_Header] = headerColor;
			ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = headerHoveredColor;
			ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = headerActiveColor;

			ImGui::End();

			m_HeaderDrawer.DrawModalWindow();
			m_TransformDrawer.DrawModalWindow();
			m_CameraDrawer.DrawModalWindow();

			if (m_CameraDrawer.removeComponent)
			{
				m_SelectedEntity.RemoveComponent<CameraComponent>();
				m_CameraDrawer.removeComponent = false;
			}
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

	void EntityInspectorEditorPanel::AddComponentMenu()
	{

		if (CustomDrawers::ButtonCenteredOnLine("Add Component", 0.5f))
		{
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (ImGui::BeginPopup("AddComponentPopup"))
		{
			ImGui::SeparatorText("Add Component");
			if (ImGui::MenuItem(" Camera Component "))
			{
				m_SelectedEntity.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}
			ImGui::Dummy(ImVec2(0.0f, 3.0f));
			ImGui::EndPopup();
		}
	}


	void HeaderDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<HeaderComponent>())
		{
			auto& header = entity.GetComponent<HeaderComponent>();

			bool open = DrawComponentHeader(m_HeaderText.c_str(), entity);

			if (open)
			{
				static char nameBuffer[256];
				std::string currentName = header.GetHeader();
				memset(nameBuffer, 0, sizeof(nameBuffer));
				strcpy_s(nameBuffer, sizeof(nameBuffer), currentName.c_str());
				nameBuffer[currentName.length()] = '\0';  // Ensure null-terminated string

				int flags = ImGuiInputTextFlags_EnterReturnsTrue;
				if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer), flags))
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
			bool open=DrawComponentHeader("Transform Component", entity);
			if (open)
			{
				CustomDrawers::DrawVector3("Position", transform.GetLocalPosition(), 80.0f, 0.0f);
				CustomDrawers::DrawVector3("Rotation", transform.GetLocalEulerAngles(), 80.0f, 0.0f);
				CustomDrawers::DrawVector3("Scale", transform.GetLocalScale(), 80.0f, 1.0f);
			}
		}
	}

	void TransformDrawer::OnSelectionChanged(Entity entity)
	{
	}

#pragma region CameraDrawer

	void CameraDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<CameraComponent>())
		{
			auto& camComponent = entity.GetComponent<CameraComponent>();
			bool open = DrawComponentHeader("Camera Component", entity);
			if (open)
			{
				auto& camera = camComponent.GetCamera();
				// Display and edit Near Plane
				float nearPlane = camera.GetNearPlane();
				if (ImGui::DragFloat("Near Plane", &nearPlane, 0.1f, 0.01f, 100.0f, "%.1f"))
				{
					camera.SetNearPlane(nearPlane);
				}

				// Display and edit Far Plane
				float farPlane = camera.GetFarPlane();
				if (ImGui::DragFloat("Far Plane", &farPlane, 0.1f, 1.0f, 10000.0f, "%.1f"))
				{
					camera.SetFarPlane(farPlane);
				}

				// Display and edit Field of View (FOV)
				float fov = camera.GetFov();
				if (ImGui::DragFloat("Field of View", &fov, 0.1f, 1.0f, 180.0f, "%.1f"))
				{
					camera.SetVerticalFov(fov);
				}

			}
		}
	}


	void CameraDrawer::OnSelectionChanged(Entity entity)
	{
	}

	void CameraDrawer::OnRemoveComponent(Entity entity)
	{
		removeComponent = true;
	}

#pragma endregion

#pragma region CustomDrawers
	void CustomDrawers::DrawVector3(const CzuchStr& label, Vec3& vec, float colWidth, float resetValue)
	{
		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, colWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = ImVec2(lineHeight + 3.0f, lineHeight);

		if (ImGui::Button("X", buttonSize))
		{
			vec.x = resetValue;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##X", &vec.x, 0.1f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (ImGui::Button("Y", buttonSize))
		{
			vec.y = resetValue;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &vec.y, 0.1f);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Z", buttonSize))
		{
			vec.z = resetValue;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &vec.z, 0.1f);
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
		ImGui::Spacing();
	}
	bool CustomDrawers::ButtonCenteredOnLine(const char* label, float alignment)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
		float avail = ImGui::GetContentRegionAvail().x;

		float off = (avail - size) * alignment;
		if (off > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

		return ImGui::Button(label);
	}
	void CustomDrawers::LabelCenteredOnLine(const char* label, float alignment,float leftPadding,float rightPadding)
	{
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(label).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Dummy(ImVec2(leftPadding, 0.0f));
		ImGui::SameLine();
		ImGui::Text(label);
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(rightPadding, 0.0f));
	}
	void CustomDrawers::ShowModalWindow(const char* title, const char* text,bool & isOpen)
	{
		ImGui::OpenPopup(title);
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			CustomDrawers::LabelCenteredOnLine(text,0.5f,5.0f,5.0f);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			if (CustomDrawers::ButtonCenteredOnLine("OK")) { ImGui::CloseCurrentPopup(); isOpen = false; }
			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			ImGui::EndPopup();
		}
	}
#pragma endregion

	bool ComponentDrawer::DrawComponentHeader(const char* name, Entity entity)
	{
		int collapseFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap;
		bool open = ImGui::CollapsingHeader(name, collapseFlags);
		ImGui::SameLine();
		ImGui::PushID(id);
		if (CustomDrawers::ButtonCenteredOnLine("+", 1.0f))
		{
			LOG_BE_INFO("Opening popup");
			ImGui::OpenPopup(name);
		}

		if (ImGui::BeginPopup(name))
		{
			ImGui::SeparatorText("Actions");
			ImGui::Dummy(ImVec2(0.0f,2.0f));
			if (ImGui::MenuItem(" Delete Component "))
			{
				OnRemoveComponent(entity);
			}
			ImGui::Dummy(ImVec2(0.0f, 2.0f));
			ImGui::Separator();
			ImGui::EndPopup();
		}
		ImGui::PopID();
		return open;
	}
	void ComponentDrawer::DrawModalWindow()
	{
		if (m_ModalData.show)
		{
			CustomDrawers::ShowModalWindow(m_ModalData.title.c_str(), m_ModalData.text.c_str(), m_ModalData.show);
		}
	}
}