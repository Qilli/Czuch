#include "czpch.h"
#include "AssetManager.h"

namespace Czuch
{
	AssetManager::AssetManager()
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

		Asset* existingRes = LoadAsset({ .handle = strId.GetGuid() });

		if (existingRes != nullptr)
		{
			return existingRes;
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

	Asset* AssetManager::LoadAsset(AssetHandle handle)
	{
		Asset* res = GetAsset({ .handle = handle.handle });

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
			res->UnloadAsset();
			//UnRegisterAsset(handle);
		}
	}

	void AssetManager::UnloadAsset(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);
		UnloadAsset({ .handle = strId.GetGuid() });
	}

	void AssetManager::UnloadAll()
	{
		for (const auto& [key, value] : m_Assets)
		{
			if (value != nullptr)
			{
				value->UnloadAsset();
			}
		}
		m_Assets.clear();
	}

	AssetHandle AssetManager::GetHandleForResource(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);
		Asset* res = GetAsset({ .handle = strId.GetGuid() });
		if (res != nullptr)
		{
			return { .handle = res->GetGuid() };
		}
		return { .handle = InvalidID };
	}

	Asset* AssetManager::GetAsset(AssetHandle handle)
	{
		auto result = m_Assets.find(handle.handle);
		if (result != m_Assets.end())
		{
			return result->second;
		}
		return nullptr;
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
