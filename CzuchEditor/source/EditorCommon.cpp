#include "EditorCommon.h"
#include"imgui.h"
#include"imgui_internal.h"
#include"Subsystems/Assets/AssetsManager.h"
#include"Subsystems/Scenes/Components/MeshComponent.h"
#include"Subsystems/Scenes/Components/MeshRendererComponent.h"
#include"Subsystems/Assets/Asset/MaterialInstanceAsset.h"
#include"Subsystems/Assets/Asset/ModelAsset.h"
#include"Subsystems/Assets/Asset/TextureAsset.h"

namespace Czuch
{

	void* EditorAssets::s_EditorFolderTexture;
	void* EditorAssets::s_EditorFileTexture;
	void* EditorAssets::s_EditorModelTexture;
	void* EditorAssets::s_EditorMaterialTexture;
	void* EditorAssets::s_EditorTextureTexture;
	void* EditorAssets::s_EditorShaderTexture;
	void* EditorAssets::s_EditorMaterialInstanceTexture;

	ImTextureID GetAssetTexturePreview(Czuch::AssetsManager* mgr, ImTextureID defaultTexture, const std::filesystem::path& path) {
		// Get the texture preview
		ImTextureID texture = defaultTexture;
		if (mgr->IsFormatAssetOfType(path.extension().string().c_str(), AssetType::TEXTURE)) {
			auto handle = mgr->LoadAsset<TextureAsset>(path.string());
			if (handle.IsValid())
			{
				auto textureAsset = mgr->GetAsset<TextureAsset>(handle);
				texture = (ImTextureID)textureAsset->GetUITextureIDPtr();
			}
			else
			{
				texture = defaultTexture;
			}
		}
		return texture;
	}


	void* EditorAssets::GetIconForType(AssetType type)
	{
			switch (type)
			{
			case AssetType::TEXTURE:
				return s_EditorTextureTexture;
			case AssetType::MATERIAL:
				return s_EditorMaterialTexture;
			case AssetType::MATERIAL_INSTANCE:
				return s_EditorMaterialInstanceTexture;
			case AssetType::MESH:
				return s_EditorModelTexture;
			case AssetType::SHADER:
				return s_EditorShaderTexture;
			default:
				return s_EditorFileTexture;
			}
	}

	bool SelectAssetHelper::ShowSelectAsset()
	{
		bool reset = false;
		float buttonHeight = ImGui::GetFrameHeight();
		float textHeight = ImGui::GetTextLineHeight();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (buttonHeight - textHeight) * 0.5f);
		ImGui::Text(m_AssetNameType.c_str());
		ImGui::SameLine();
		if (HasSelectedAsset())
		{
			auto name = GetSelectedAssetName();
			ImGui::Text(name!=nullptr?name->c_str() : "Missing");
		}
		else
		{
			ImGui::Text("None");
		}

		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (buttonHeight - textHeight) * 0.5f);
		if (ImGui::Button(m_AssetSelectButtonID.c_str())) {
			ImGui::OpenPopup(m_AssetPopupName.c_str());
			m_AssetSelectorPopup.filterOnEnter = true;
		}

		auto selected = m_AssetSelectorPopup.ShowAssetSelectorPopup(m_AssetPopupName.c_str(), false);

		if (selected != nullptr && !ImGui::IsPopupOpen(m_AssetPopupName.c_str()))
		{
			SetSelected(selected->asset,selected->resource);
			m_AssetSelectorPopup.selected = nullptr;
		}
		return reset;
	}

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
	bool CustomDrawers::DrawColorWithLabel(const CzuchStr& label, Vec4& color, float colWidth)
	{
		bool valueChanged = false;

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, colWidth);
		ImGui::Text("%s", label.c_str());
		ImGui::NextColumn();

		valueChanged = ImGui::ColorEdit4("##ColorPicker", &color.x, ImGuiColorEditFlags_NoInputs);

		ImGui::Columns(1);
		ImGui::PopID();

		return valueChanged;
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


	void CustomDrawers::FormatAssetName(Czuch::ShortAssetInfo* const asset,char* tempName)
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

	bool CustomDrawers::DrawLinearColor(const CzuchStr& name, Vec4* color)
	{
		bool changed = false;
		ImGui::PushID(name.c_str());
		Vec4 gammaColor = LinearToGamma(*color);
		if (ImGui::ColorEdit4(name.c_str(), &gammaColor.x, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_NoOptions))
		{
			changed = true;
			*color = GammaToLinear(gammaColor);
		}

		ImGui::PopID();
		return changed;
	}


#pragma endregion

	bool SelectMeshAssetHelper::HasSelectedAsset()
	{
		return m_MeshComponent != nullptr && m_MeshComponent->HasMesh();
	}

	const CzuchStr* SelectMeshAssetHelper::GetSelectedAssetName()
	{
		if (m_MeshComponent != nullptr && m_MeshComponent->HasMesh())
		{
			auto modelAsset = AssetsManager::GetPtr()->GetAsset<ModelAsset>(m_MeshComponent->GetModel());
			auto name = modelAsset->GetMeshName(m_MeshComponent->GetMesh());
			return name;
		}
		return nullptr;
	}

	void SelectMeshAssetHelper::SetSelected(AssetHandle asset, I32 resource)
	{
		if (m_MeshComponent != nullptr)
		{
			m_MeshComponent->SetMesh(asset, MeshHandle(resource));
		}
	}

	bool SelectMaterialInstanceAssetHelper::HasSelectedAsset()
	{
		return HANDLE_IS_VALID(m_MeshRendererComponent->GetOverrideMaterial());
	}

	const CzuchStr* SelectMaterialInstanceAssetHelper::GetSelectedAssetName()
	{
		auto matHandle=m_MeshRendererComponent->GetOverrideMaterial();
		if (HANDLE_IS_VALID(matHandle))
		{
			auto matAsset = AssetsManager::GetPtr()->GetAsset<MaterialInstanceAsset>(m_MeshRendererComponent->GetMaterialAsset());
			auto &name = matAsset->AssetName();
			return &name;
		}
		return nullptr;
	}

	void SelectMaterialInstanceAssetHelper::SetSelected(AssetHandle asset, I32 resource)
	{
		m_MeshRendererComponent->SetOverrideMaterial(asset);
	}

	void ShowAssetSelectorPopupHelper::Reset()
	{
		memset(searchBuffer, 0, sizeof(searchBuffer));
		filterOnEnter = true;
	}

	const ShortAssetInfo* ShowAssetSelectorPopupHelper::ShowAssetSelectorPopup(const char* popupId, bool canChangeType)
	{
		ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_Appearing);
		if (ImGui::BeginPopupModal(popupId, nullptr))
		{
			if (filterOnEnter)
			{
				filteredAssets = &AssetsManager::GetPtr()->GetAssetsOfTypes(selectedType);
				filterOnEnter = false;
				selected = nullptr;
			}

			// Search Bar
			ImGui::InputTextWithHint("##Search", "Search...", searchBuffer, IM_ARRAYSIZE(searchBuffer));

			ImGui::SameLine();
			ImGui::Text("Type:");
			ImGui::SameLine();

			// Dropdown for type selection
			if (!canChangeType)
			{
				ImGui::BeginDisabled(!canChangeType);
			}

			if (ImGui::BeginCombo("##Type", s_AssetTypesNames[selectedType])) {
				for (int i = 0; i < IM_ARRAYSIZE(s_AssetTypesNames); i++) {
					if (ImGui::Selectable(s_AssetTypesNames[i], selectedType == i)) {
						selectedType = (AssetType)i;
						filteredAssets = &AssetsManager::GetPtr()->GetAssetsOfTypes(selectedType);
					}
				}
				ImGui::EndCombo();
			}

			if (!canChangeType)
			{
				ImGui::EndDisabled();
			}

			ImGui::Separator();
			bool closePopup = false;

			// Begin a child region to make the table scrollable
			if (ImGui::BeginChild("AssetsList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 10), true))
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

						ImTextureID texture = (ImTextureID)EditorAssets::GetIconForType(asset->type);

						if (asset->type == AssetType::TEXTURE)
						{
							std::filesystem::path path = AssetsManager::GetPtr()->GetAssetPath(asset->asset);
							texture = GetAssetTexturePreview(AssetsManager::GetPtr(), texture,std::filesystem::relative( path,AssetsManager::GetStartPath()));
						}


						if (ImGui::ImageButton(buttonID.c_str(), texture, ImVec2(thumbnailSize, thumbnailSize)))
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

						CustomDrawers::FormatAssetName(asset,tempName);

						// Display the name
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellSize - thumbnailSize) / 2);
						ImGui::TextWrapped("%s", tempName);

						ImGui::EndGroup();

						columnIndex = (columnIndex + 1) % columns;
					}
					ImGui::EndTable();
				}

				if (filteredAssets->empty()) {
					ImGui::Text("No assets found.");
				}

				ImGui::EndChild();
			}

			ImGui::Separator();

			//Select button
			if (ImGui::Button("Select") || closePopup) {
				if (selected != nullptr)
				{
					Reset();
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::SameLine();

			// Cancel Button
			if (ImGui::Button("Cancel")) {
				Reset();
				ImGui::CloseCurrentPopup();
				selected = nullptr;
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

	void SelectTextureAssetHelper::SetMaterialInstance(MaterialInstanceAsset* mat, int index)
	{
		m_MaterialInstance = mat;
		m_ParamIndex = index;
		m_AssetNameType = " "+m_MaterialInstance->GetParameterAtIndexName(index)+": ";
	}

	bool SelectTextureAssetHelper::HasSelectedAsset()
	{
		if (m_ParamIndex >= 0)
		{
			auto [asset, resource] = m_MaterialInstance->GetTextureAssetAtIndex(m_ParamIndex);
			return HANDLE_IS_VALID(asset);
		}
		return false;
	}

	const CzuchStr* SelectTextureAssetHelper::GetSelectedAssetName()
	{
		if (m_ParamIndex >= 0)
		{
			auto [asset, resource] = m_MaterialInstance->GetTextureAssetAtIndex(m_ParamIndex);
			if (HANDLE_IS_VALID(asset))
			{
				auto textureAsset = AssetsManager::GetPtr()->GetAsset<TextureAsset>(asset);
				auto &name = textureAsset->AssetName();
				return &name;
			}
		}
		return nullptr;
	}

	void SelectTextureAssetHelper::SetSelected(AssetHandle asset, I32 resource)
	{
		if (m_ParamIndex >= 0)
		{
			m_MaterialInstance->SetTextureParameterAtIndex(m_ParamIndex, asset, resource);
		}
	}

}
