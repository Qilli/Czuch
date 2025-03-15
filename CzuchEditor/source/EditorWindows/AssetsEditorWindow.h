#pragma once
#include "BaseEditorWindow.h"
#include<filesystem>

namespace fs = std::filesystem;

namespace Czuch
{

	struct FileEntry {
		std::string name;
		std::filesystem::path path;
		bool isDirectory;
		void* thumbnail; 
	};

	struct AssetMetaEditorEntry
	{
		std::filesystem::path path;
		bool exist;
	};

	class Renderer;
    class AssetsEditorWindow :
        public BaseEditorWindow
    {
	public:
		AssetsEditorWindow(const char* name);
		virtual ~AssetsEditorWindow();
		virtual void FillUI() override;
		virtual void Init(EngineRoot* root) override;
	private:
		void ComputeAssetsDiff();
		void FolderStructureChanged(const std::string& startPath);
		void CheckAsset(const std::filesystem::path& path);
	private:
		std::filesystem::path m_StartPath;
		std::filesystem::path m_CurrentPath;
		std::vector<FileEntry> entries;
		std::vector<AssetMetaEditorEntry> allFilesEntries;
		std::vector<std::filesystem::path> newToAdd;
		std::vector<std::filesystem::path> toRemove;
		float m_FileStructureCheckTime = 1000.0f;
		float m_LastFileStructureCheckTime = 0.0f;
		const FileEntry* m_SelectedEntry = nullptr;
    };



}


