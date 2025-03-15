#pragma once
#include"Core/EngineCore.h"
#include"Asset/Asset.h"
#include<unordered_map>
#include"SettingsPerType.h"

namespace Czuch
{
	class AssetManager
	{
	public:
		friend class AssetsManager;
		AssetManager();
		virtual ~AssetManager();
		virtual Asset* LoadAsset(const CzuchStr& path,BaseLoadSettings& settings);
		virtual void TryCreateAssetWithUnloadState(const CzuchStr& path);
		virtual Asset* LoadAsset(AssetHandle handle);
		virtual void UnloadAsset(AssetHandle handle);
		virtual void RemoveAsset(AssetHandle handle);
		virtual void UnloadAsset(const CzuchStr& path);
		virtual void UnloadAll();
		virtual AssetHandle GetHandleForResource(const CzuchStr& path);
		virtual Asset* GetAsset(AssetHandle handle);
		virtual bool IsFormatSupported(const char* format) const = 0;
		Asset* GetDefaultAsset() { return m_DefaultAsset; }
		auto GetAssetIterator() const { return m_Assets.begin(); }
		auto GetAssetEnd() const { return m_Assets.end(); }
		void IncrementAssetRef(AssetHandle handle);
		virtual int GetAssetType() const = 0;
		bool AssetExistAtPath(const CzuchStr& path);
	public:
		virtual void Init() = 0;
	protected:
		virtual Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) = 0;
		virtual Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) = 0;
		virtual Asset* CreateLoadableAsset(const CzuchStr& path) = 0;
	protected:
		void RegisterAsset(StringID& strId, Asset* createdRes);
		void UnRegisterAsset(AssetHandle handle);
	protected:
		Asset* m_DefaultAsset;
	private:
		std::unordered_map<Guid, Asset*> m_Assets;
	};
}

