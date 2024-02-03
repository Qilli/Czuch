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

	Asset* AssetManager::LoadAsset(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);

		Asset* existingRes = LoadAsset({ .id = strId.GetGuid() });

		if (existingRes != nullptr)
		{
			return existingRes;
		}

		Asset* createdRes = CreateAsset(path,m_Settings);
		m_Assets.insert({ strId.GetGuid(), createdRes });

		auto result = createdRes->LoadAsset();

		if (result == false)
		{
			return nullptr;
		}
		return createdRes;
	}

	Asset* AssetManager::LoadAsset(AssetHandle handle)
	{
		Asset* res = GetAsset({ .id = handle.id });

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

	void AssetManager::SetSettings(void* settings)
	{
		m_Settings = settings;
	}

	void AssetManager::UnloadAsset(AssetHandle handle)
	{
		Asset* res = GetAsset(handle);
		if (res != nullptr)
		{
			res->UnloadAsset();
		}
	}

	void AssetManager::UnloadAsset(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);
		UnloadAsset({ .id = strId.GetGuid() });
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
	}

	AssetHandle AssetManager::GetHandleForResource(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);
		Asset* res = GetAsset({ .id = strId.GetGuid() });
		if (res != nullptr)
		{
			return { .id = res->GetGuid() };
		}
		return { .id = InvalidID };
	}

	Asset* AssetManager::GetAsset(AssetHandle handle)
	{
		auto result = m_Assets.find(handle.id);
		if (result != m_Assets.end())
		{
			return result->second;
		}
		return nullptr;
	}

}
