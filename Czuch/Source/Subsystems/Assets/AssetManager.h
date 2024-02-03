#pragma once
#include"Core/EngineCore.h"
#include"Asset/Asset.h"
#include<unordered_map>

namespace Czuch
{
	class AssetManager
	{
	public:
		AssetManager();
		virtual ~AssetManager();
		virtual Asset* LoadAsset(const CzuchStr& path);
		virtual Asset* LoadAsset(AssetHandle handle);
		virtual void SetSettings(void* settings);
		virtual void UnloadAsset(AssetHandle handle);
		virtual void UnloadAsset(const CzuchStr& path);
		virtual void UnloadAll();
		virtual AssetHandle GetHandleForResource(const CzuchStr& path);
		virtual Asset* GetAsset(AssetHandle handle);
	protected:
		virtual Asset* CreateAsset(const CzuchStr& path,void* settings)=0;
	private:
		std::unordered_map<Guid, Asset*> m_Assets;
		void* m_Settings;
	};
}

