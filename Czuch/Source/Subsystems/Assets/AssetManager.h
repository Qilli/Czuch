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
		AssetManager();
		virtual ~AssetManager();
		virtual Asset* LoadAsset(const CzuchStr& path,BaseLoadSettings& settings);
		virtual Asset* LoadAsset(AssetHandle handle);
		virtual void UnloadAsset(AssetHandle handle);
		virtual void UnloadAsset(const CzuchStr& path);
		virtual void UnloadAll();
		virtual AssetHandle GetHandleForResource(const CzuchStr& path);
		virtual Asset* GetAsset(AssetHandle handle);
		virtual Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings)=0;
		virtual Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settingsa)=0;
	protected:
		void RegisterAsset(StringID& strId, Asset* createdRes);
		void UnRegisterAsset(AssetHandle handle);
	private:
		std::unordered_map<Guid, Asset*> m_Assets;
	};
}

