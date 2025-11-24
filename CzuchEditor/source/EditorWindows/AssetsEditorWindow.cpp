#include "AssetsEditorWindow.h"
#include"imgui.h"
#include "Czuch.h"
#include"Core/Time.h"
#include"../EditorCommon.h"


namespace Czuch
{
	static const float THUMBNAIL_SIZE = 64.0f;
	static float PADDING = 16.0f;
	static float CELL_SIZE = THUMBNAIL_SIZE + PADDING;

	bool IsEditorFolder(const std::filesystem::path& path)
	{
		std::string folderName = path.filename().string();

		if (folderName == "Editor" || folderName == "editor")
		{
			return true;
		}
		return false;
	}


	void LoadFolderContents(Czuch::AssetsManager* mgr,const fs::path& path,const fs::path& startPath, std::vector<FileEntry>& entries, ImTextureID folderTexture, ImTextureID fileTexture) {
		entries.clear();
		for (const auto& entry : fs::directory_iterator(path)) {

			if (entry.is_directory() && IsEditorFolder(entry.path()))
			{
				continue;
			}

			if (!entry.is_directory() && !mgr->IsFormatSupported(entry.path().extension().string().c_str()))
			{
				continue;
			}

			ImTextureID thumbnail = entry.is_directory() ? folderTexture : GetAssetTexturePreview(mgr,fileTexture, fs::relative(entry.path(), startPath));

			entries.push_back({
				entry.path().filename().string(),
				entry.path(),
				entry.is_directory(),
				(void*)thumbnail
				});
		}
	}

	AssetsEditorWindow::AssetsEditorWindow(const char* name) :BaseEditorWindow()
	{
		SetWindowName(name);
	}

	AssetsEditorWindow::~AssetsEditorWindow()
	{
	}

	void AssetsEditorWindow::FillUI()
	{
		if (Czuch::AssetsManager::GetPtr()->IsDuringShutdown())
		{
			return;
		}

		if (m_LastFileStructureCheckTime + m_FileStructureCheckTime < Time::TimeFromStart())
		{
			ComputeAssetsDiff();
		}


		auto mgr = AssetsManager::GetPtr();
		// Load folder contents if the directory changes
		LoadFolderContents(mgr,m_CurrentPath,m_StartPath, entries, (ImTextureID)EditorAssets::s_EditorFolderTexture, (ImTextureID)EditorAssets::s_EditorFileTexture);

		// Display the current path and a button to go up one level
		ImGui::Text("Current Path: %s", m_CurrentPath.string().c_str());
		if (ImGui::Button("Up") && m_CurrentPath.has_parent_path()) {
			if (m_CurrentPath != m_StartPath)
			{
				m_CurrentPath = m_CurrentPath.parent_path();
			}
			return;
		}

		// Define the grid parameters
		const float thumbnailSize = THUMBNAIL_SIZE;
		const float padding = PADDING;
		const float cellSize = CELL_SIZE;
		const float windowWidth = ImGui::GetContentRegionAvail().x;
		int columns = static_cast<int>(windowWidth / cellSize);
		if (columns < 1) columns = 1;

		// Make the grid scrollable
		ImGui::BeginChild("FolderViewerChild", ImVec2(0, 0), true);

		// Create the table for the grid
		if (ImGui::BeginTable("FolderGrid", columns, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoPadOuterX)) {
			int columnIndex = 0;
			U32 i = 0;
			for (const auto& entry : entries) {
				if (columnIndex == 0) {
					ImGui::TableNextRow();
				}

				ImGui::TableNextColumn();

				ImVec2 min = ImGui::GetCursorScreenPos();// Top-left corner of the current cell
				// Center the thumbnail and name
				ImGui::BeginGroup();

				// Display the thumbnail
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellSize - thumbnailSize) / 2);
				std::string buttonID = "##Thumbnail" + std::to_string(i++);
				ImGui::ImageButton(buttonID.c_str(), (ImTextureID)entry.thumbnail, ImVec2(thumbnailSize, thumbnailSize));

				if (ImGui::BeginDragDropSource())
				{
					std::filesystem::path relativePath = fs::relative(entry.path, m_StartPath);
					auto assetPath = relativePath.c_str();
					ImGui::SetDragDropPayload("ASSETS_EDITOR_ASSET",assetPath, strlen(assetPath) * sizeof(char) + 1 ,ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					m_SelectedEntry = &entry;
					if (entry.isDirectory) {
						m_CurrentPath /= entry.name;
					}
					else {
						//we are clicking a file not a folder
					}
				}

				// Display the name
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellSize - thumbnailSize) / 2);
				ImGui::TextWrapped("%s", entry.name.c_str());

				ImGui::EndGroup();

				ImVec2 max = ImVec2(min.x+cellSize,min.y+cellSize*0.9f);
				if (m_SelectedEntry != nullptr && entry.path == m_SelectedEntry->path) {
					ImGui::GetWindowDrawList()->AddRect(
						min, max,                           // Top-left and bottom-right corners
						IM_COL32(0, 255, 0, 255),           // Green color (RGBA)
						5.0f,                               // Optional rounding
						0,                                  // Flags (0 for solid border)
						2.0f                                // Border thickness
					);
				}

				columnIndex = (columnIndex + 1) % columns;
			}
			ImGui::EndTable();
		}

		ImGui::EndChild();
	}


	void AssetsEditorWindow::Init(EngineRoot* root)
	{
		std::filesystem::path path = std::filesystem::current_path();
		path.append("Assets");
		m_StartPath = path;

		m_CurrentPath = m_StartPath;
		m_SelectedEntry = nullptr;

		auto mgr = AssetsManager::GetPtr();

		ComputeAssetsDiff();
	}

	void AssetsEditorWindow::ComputeAssetsDiff()
	{
		newToAdd.clear();
		toRemove.clear();

		for (auto& entry : allFilesEntries)
		{
			entry.exist = false;
		}

		FolderStructureChanged(m_StartPath.string());

		for (auto& entry : allFilesEntries)
		{
			if (!entry.exist)
			{
				toRemove.push_back(entry.path);
				if (m_SelectedEntry != nullptr && m_SelectedEntry->path == entry.path)
				{
					m_SelectedEntry = nullptr;
				}
			}
		}

		for (auto& path : newToAdd)
		{
			AssetsManager::GetPtr()->CheckIfAssetExistsAndIfNotCreate(path);
			allFilesEntries.push_back({ path,true });
		}

		for (auto& path : toRemove)
		{
			allFilesEntries.erase(std::remove_if(allFilesEntries.begin(), allFilesEntries.end(), [path](const AssetMetaEditorEntry& entry) {
				return entry.path == path;
				}), allFilesEntries.end());
			AssetsManager::GetPtr()->UnloadAndRemoveAsset(path.string());
		}

		m_LastFileStructureCheckTime = Time::TimeFromStart();
	}

	void AssetsEditorWindow::FolderStructureChanged(const std::string& startPath)
	{
		auto mgr = AssetsManager::GetPtr();

		for (const auto& entry : fs::directory_iterator(startPath)) {

			if (entry.is_directory() && IsEditorFolder(entry.path()))
			{
				continue;
			}

			if (entry.is_directory())
			{
				FolderStructureChanged(entry.path().string());
			}

			if (!entry.is_directory() && mgr->IsFormatSupported(entry.path().extension().string().c_str()))
			{
				CheckAsset(fs::relative(entry.path(),m_StartPath));
			}
		}
	}

	void AssetsEditorWindow::CheckAsset(const std::filesystem::path& path)
	{
		auto mgr = AssetsManager::GetPtr();
		auto result = std::find_if(allFilesEntries.begin(), allFilesEntries.end(), [path](const AssetMetaEditorEntry& entry) {
			return entry.path == path;
			});

		if (result == allFilesEntries.end())
		{
			newToAdd.push_back(path);
		}
		else
		{
			result->exist = true;
		}
	}
}