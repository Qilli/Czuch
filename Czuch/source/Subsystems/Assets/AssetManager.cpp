#include "czpch.h"
#include "AssetManager.h"
#include "AssetsManager.h"

namespace Czuch
{
	AssetManager::AssetManager() : m_DefaultAsset(nullptr)
	{
	}

	AssetManager::~AssetManager()
	{
		for(auto res : m_Assets)
		{
			if (res.second != nullptr)
			{
				delete res.second;
			}
		}
		m_Assets.clear();
	}

	Asset* AssetManager::LoadAsset(const CzuchStr& path,BaseLoadSettings& settings)
	{
		StringID strId = StringID::MakeStringID(path);

		Asset* existingRes = LoadAsset({ strId.GetGuid()});

		if (existingRes != nullptr)
		{
			return existingRes;
		}

		//check if asset exist
		if (!AssetExistAtPath(path))
		{
			LOG_BE_ERROR("Asset with path: {0} doesn't exist!", path);
			return nullptr;
		}

		Asset* createdRes = CreateAsset(path,settings);
		RegisterAsset(strId, createdRes);

		auto result = createdRes->LoadAsset();

		if (result == false)
		{
			return nullptr;
		}
		return createdRes;
	}

	void AssetManager::TryCreateAssetWithUnloadState(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);

		Asset* res = GetAsset({strId.GetGuid()});

		if (res != nullptr)
		{
			return;
		}

		Asset* createdRes = CreateLoadableAsset(path);
		RegisterAsset(strId, createdRes);
	}

	Asset* AssetManager::LoadAsset(AssetHandle handle)
	{
		Asset* res = GetAsset({handle.handle});

		if (res != nullptr)
		{
			bool loaded = res->LoadAsset();
			if (loaded == true)
			{
				return res;
			}
			return nullptr;
		}
		return nullptr;
	}

	void AssetManager::UnloadAsset(AssetHandle handle)
	{
		Asset* res = GetAsset(handle);
		if (res != nullptr)
		{
			if (res->UnloadAsset())
			{
				LOG_BE_INFO("Asset with name {0} was unloaded", res->AssetName());
			}
		}
	}

	void AssetManager::RemoveAsset(AssetHandle handle)
	{
		Asset* res = GetAsset(handle);
		if (res != nullptr)
		{
			res->UnloadAsset();
			UnRegisterAsset(handle);
		}
	}

	void AssetManager::UnloadAsset(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);
		UnloadAsset(strId.GetGuid());
	}

	void AssetManager::UnloadAll()
	{
		for (const auto& [key, value] : m_Assets)
		{
			if (value != nullptr)
			{
				value->ForceUnload();
				if (value->UnloadAsset())
				{
					LOG_BE_INFO("Asset with name {0} was unloaded on unload all", value->AssetName());
				}
			}
		}
	}

	AssetHandle AssetManager::GetHandleForResource(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);
		Asset* res = GetAsset( {strId.GetGuid()});
		if (res != nullptr)
		{
			return {res->GetGuid()};
		}
		return AssetHandle();
	}

	Asset* AssetManager::GetAsset(AssetHandle handle)
	{
		auto result = m_Assets.find(handle.handle);
		if (result != m_Assets.end())
		{
			auto res = result->second;

			if (res != nullptr)
			{
				if (!res->IsLoaded())
				{
					res->LoadAsset();
				}
				return res;
			}
		}
		return nullptr;
	}

	void AssetManager::IncrementAssetRef(AssetHandle handle)
	{
		Asset* res = GetAsset(handle);
		if (res != nullptr)
		{
			res->IncrementRefCounter();
		}
	}

	bool AssetManager::AssetExistAtPath(const CzuchStr& path)
	{
		return std::filesystem::exists(AssetsManager::GetStartPath()+path);
	}

	const Array<Asset*>& AssetManager::GetAllAssetsWithFilter(std::function<bool(Asset*)> predicate)
	{
		m_filterArray.clear();
		for(auto &asset : m_Assets)
		{
			if (predicate(asset.second))
			{
				m_filterArray.push_back(asset.second);
			}
		}
		return m_filterArray;
	}

	void AssetManager::ExecuteOnAllAssets(std::function<void(Asset*)> func)
	{
		for (auto& asset : m_Assets)
		{
			func(asset.second);
		}
	}

	void AssetManager::RegisterAsset(StringID& strId, Asset* createdRes)
	{
		m_Assets.insert({ strId.GetGuid(), createdRes });
	}

	void AssetManager::UnRegisterAsset(AssetHandle handle)
	{
		m_Assets.erase(handle.handle);
	}

}
