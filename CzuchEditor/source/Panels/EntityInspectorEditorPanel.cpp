#include "BaseEditorPanel.h"
#include "EntityInspectorEditorPanel.h"
#include"imgui.h"
#include"imgui_internal.h"
#include"Subsystems/Scenes/Components/HeaderComponent.h"
#include"Subsystems/Scenes/Components/TransformComponent.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include"Subsystems/Scenes/Components/MeshComponent.h"
#include"Subsystems/Scenes/Components/MeshRendererComponent.h"
#include"Subsystems/Assets/AssetsManager.h"
#include"Subsystems/Assets/Asset/ModelAsset.h"
#include"../Commands/CommandTypes/ChangeTransformCommand.h"
#include"../Commands/EditorCommandsControl.h"
#include"../EditorCommon.h"

namespace Czuch
{

	bool CustomDrawers::m_ResetShowAssetModal = false;
	char CustomDrawers::tempName[15] = { 0 };

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
			m_MeshDrawer.DrawComponent(m_SelectedEntity);
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
			m_MeshDrawer.DrawModalWindow();

			if (m_CameraDrawer.removeComponent)
			{
				m_SelectedEntity.RemoveComponent<CameraComponent>();
				m_CameraDrawer.removeComponent = false;
			}

			if (m_MeshDrawer.removeComponent)
			{
				m_SelectedEntity.RemoveComponent<MeshComponent>();
				m_MeshDrawer.removeComponent = false;
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

			if (ImGui::MenuItem(" Mesh Component "))
			{
				m_SelectedEntity.AddComponent<MeshComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem(" Mesh Renderer Component "))
			{
				m_SelectedEntity.AddComponent<MeshRendererComponent>();
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
			bool open = DrawComponentHeader("Transform Component", entity);
			if (open)
			{
				bool changed = false;

				if (CustomDrawers::DrawVector3("Position", transform.GetLocalPosition(), 80.0f, 0.0f, m_Position))
				{
					transform.ForceUpdateLocalTransform();
					changed = true;
				}
				if (CustomDrawers::DrawVector3("Rotation", transform.GetLocalEulerAngles(), 80.0f, 0.0f, m_Rotation))
				{
					transform.ForceUpdateLocalTransform();
					changed = true;
				}
				if (CustomDrawers::DrawVector3("Scale", transform.GetLocalScale(), 80.0f, 1.0f, m_Scale))
				{
					transform.ForceUpdateLocalTransform();
					changed = true;
				}

				if (changed)
				{
					auto command = NEW(ChangeTransformCommand((Scene*)entity.GetScene(), entity, m_Position.value, m_Rotation.value, m_Scale.value));
					m_Position.isLocked = false;
					m_Rotation.isLocked = false;
					m_Scale.isLocked = false;
					m_Position.value = transform.GetLocalPosition();
					m_Rotation.value = transform.GetLocalEulerAngles();
					m_Scale.value = transform.GetLocalScale();
					EditorCommandsControl::Get().ExecuteCommand(command);
				}
				transform.ForceUpdateLocalTransform();
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
	bool CustomDrawers::DrawVector3(const CzuchStr& label, Vec3& vec, float colWidth, float resetValue, LockedVec3& locked)
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
		bool changed = false;

		if (ImGui::Button("X", buttonSize))
		{
			locked.isLocked = true;
			locked.value = vec;
			vec.x = resetValue;
			changed = true;
		}
		ImGui::SameLine();

		ImGui::DragFloat("##X", &vec.x, 0.1f);
		if (ImGui::IsItemActivated() && locked.isLocked == false)
		{
			locked.value = vec;
			locked.isLocked = true;
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			changed = true;
		}

		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (ImGui::Button("Y", buttonSize))
		{
			locked.isLocked = true;
			locked.value = vec;
			vec.y = resetValue;
			changed = true;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &vec.y, 0.1f);
		if (ImGui::IsItemActivated() && locked.isLocked == false)
		{
			locked.value = vec;
			locked.isLocked = true;
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			changed = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Z", buttonSize))
		{
			locked.isLocked = true;
			locked.value = vec;
			vec.z = resetValue;
			changed = true;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &vec.z, 0.1f);
		if (ImGui::IsItemActivated() && locked.isLocked == false)
		{
			locked.value = vec;
			locked.isLocked = true;
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			changed = true;
		}
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
		ImGui::Spacing();
		return changed;
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
	void CustomDrawers::LabelCenteredOnLine(const char* label, float alignment, float leftPadding, float rightPadding)
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
	void CustomDrawers::ShowModalWindow(const char* title, const char* text, bool& isOpen)
	{
		ImGui::OpenPopup(title);
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			CustomDrawers::LabelCenteredOnLine(text, 0.5f, 5.0f, 5.0f);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			if (CustomDrawers::ButtonCenteredOnLine("OK")) { ImGui::CloseCurrentPopup(); isOpen = false; }
			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			ImGui::EndPopup();
		}
	}



	const ShortAssetInfo* CustomDrawers::ShowAssetSelectorPopup(const char* popupId, AssetType filterSearchType,bool changeTypeActive)
	{
		static char searchBuffer[128] = "";
		static const char* assetTypes[] = {
			"Texture", "Material", "MaterialInstance", "Mesh", "Shader","All"
		};

		static AssetType selectedType = filterSearchType;
		static const Array<ShortAssetInfo*>* filteredAssets;
		static const ShortAssetInfo* selected;

		if (m_ResetShowAssetModal)
		{
			m_ResetShowAssetModal = false;
			memset(searchBuffer, 0, sizeof(searchBuffer));
			selectedType = filterSearchType;
			filteredAssets = &AssetsManager::GetPtr()->GetAssetsOfTypes(filterSearchType);
			selected = nullptr;
		}

		ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_Appearing);

		if (ImGui::BeginPopupModal(popupId, nullptr))
		{
			// Search Bar
			ImGui::InputTextWithHint("##Search", "Search...", searchBuffer, IM_ARRAYSIZE(searchBuffer));

			ImGui::SameLine();
			ImGui::Text("Type:");
			ImGui::SameLine();

			// Dropdown for type selection
			if (!changeTypeActive)
			{
				ImGui::BeginDisabled(!changeTypeActive);
			}

			if (ImGui::BeginCombo("##Type", assetTypes[selectedType])) {
					for (int i = 0; i < IM_ARRAYSIZE(assetTypes); i++) {
						if (ImGui::Selectable(assetTypes[i], selectedType == i)) {
							selectedType = (AssetType)i;
							// Update the filtered assets based on the new type
							filteredAssets = &AssetsManager::GetPtr()->GetAssetsOfTypes(selectedType);
						}
					}
				ImGui::EndCombo();
			}

			if (!changeTypeActive)
			{
				ImGui::EndDisabled();
			}

			ImGui::Separator();
			bool closePopup = false;

			// Begin a child region to make the table scrollable
			if (ImGui::BeginChild("AssetsList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 10), true, ImGuiWindowFlags_AlwaysVerticalScrollbar))
			{
				const float thumbnailSize = 80;
				const float padding = 20;
				const float cellSize = 100; // Adjusted for better alignment
				const float windowWidth = ImGui::GetContentRegionAvail().x;
				int columns = static_cast<int>(windowWidth / cellSize);
				if (columns < 1) columns = 1;

				auto assetsRef = *filteredAssets;

				if (ImGui::BeginTable("AssetsList", columns, ImGuiTableFlags_BordersInner | ImGuiTableFlags_ScrollX))
				{
					int columnIndex = 0;
					U32 i = 0;
					for (const auto asset : assetsRef) {
						if (columnIndex == 0) {
							ImGui::TableNextRow();
						}

						ImGui::TableNextColumn();

						// Center the thumbnail and name
						ImGui::BeginGroup();

						// Display the thumbnail
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellSize - thumbnailSize) / 2);
						std::string buttonID = "##Thumbnail" + std::to_string(i++);


						bool isSelected = (selected == asset);

						if (isSelected) {
							// Push a different style for the selected button
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f)); // Blue background
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f)); // Lighter blue when hovered
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.6f, 0.9f, 1.0f)); // Same blue when active
						}


						if(ImGui::ImageButton(buttonID.c_str(), (ImTextureID)EditorAssets::GetIconForType(asset->type), ImVec2(thumbnailSize, thumbnailSize)))
						{
							if (isSelected)
							{
								closePopup = true;
							}
							selected = asset;
						}

						if (isSelected) {
							ImGui::PopStyleColor(3);
						}

						FormatAssetName(asset);

						// Display the name
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellSize - thumbnailSize) / 2);
						ImGui::TextWrapped("%s", tempName);

						ImGui::EndGroup();

						columnIndex = (columnIndex + 1) % columns;
					}
					ImGui::EndTable();
				}

				// If no results
				if (filteredAssets->empty()) {
					ImGui::Text("No assets found.");
				}

				ImGui::EndChild(); // End scrollable asset list
			}


			ImGui::Separator();

			//Select button
			if (ImGui::Button("Select") || closePopup) {
				if (selected != nullptr)
				{
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::SameLine();

			// Cancel Button
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (selected != nullptr)
			{
				ImGui::Text("Selected: %s", selected->name->c_str());
			}

			ImGui::EndPopup();
		}
		return selected;
	}

	void CustomDrawers::FormatAssetName(Czuch::ShortAssetInfo* const asset)
	{
		if (asset->name->length() > 14) {
			strncpy(tempName, asset->name->c_str(), 12);
			tempName[12] = '.';
			tempName[13] = '.';
		}
		else {
			strncpy(tempName, asset->name->c_str(), asset->name->length());
			tempName[asset->name->length()] = '\0';
		}

		tempName[14] = '\0';
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
			ImGui::Dummy(ImVec2(0.0f, 2.0f));
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


#pragma region MeshComponent
	void MeshInspectorDrawer::DrawComponent(Entity entity)
	{
		if (entity.HasComponent<MeshComponent>())
		{
			auto& meshComponent = entity.GetComponent<MeshComponent>();
			bool open = DrawComponentHeader("Mesh Component", entity);
			if (open)
			{
				float buttonHeight = ImGui::GetFrameHeight();
				float textHeight = ImGui::GetTextLineHeight();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (buttonHeight - textHeight) * 0.5f);
				ImGui::Text("  Mesh:");
				ImGui::SameLine();
				if (meshComponent.HasMesh())
				{
					auto mesh = meshComponent.GetMesh();
					auto modelAsset = AssetsManager::GetPtr()->GetAsset<ModelAsset>(meshComponent.GetModel());
					auto name = modelAsset->GetMeshName(mesh);
					ImGui::Text("%s", name != nullptr ? name->c_str() : "Missing");
				}
				else
				{
					ImGui::Text("None");
				}
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (buttonHeight - textHeight) * 0.5f);
				if (ImGui::Button("Select")) {
					ImGui::OpenPopup("MeshSelectorPopup");
					CustomDrawers::m_ResetShowAssetModal = true;
				}

				auto selected=CustomDrawers::ShowAssetSelectorPopup("MeshSelectorPopup", AssetType::MESH,false);

				if (selected != nullptr && !ImGui::IsPopupOpen("MeshSelectorPopup"))
				{
					meshComponent.SetMesh(selected->asset, MeshHandle(selected->resource));
				}
			}
		}
	}
	void MeshInspectorDrawer::OnSelectionChanged(Entity entity)
	{

	}
	void MeshInspectorDrawer::OnRemoveComponent(Entity entity)
	{
		removeComponent = true;
	}
#pragma endregion
}