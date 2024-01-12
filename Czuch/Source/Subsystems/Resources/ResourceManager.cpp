#include "czpch.h"
#include "ResourceManager.h"

namespace Czuch
{
	ResourceManager::ResourceManager()
	{
	}

	ResourceManager::~ResourceManager()
	{
		for(auto res : m_Resources)
		{
			if (res.second != nullptr)
			{
				delete res.second;
			}
		}
		m_Resources.clear();
	}

	Resource* ResourceManager::LoadResource(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);

		Resource* existingRes = LoadResource({ .id = strId.GetGuid() });

		if (existingRes != nullptr)
		{
			return existingRes;
		}

		Resource* createdRes = CreateResource(path);
		m_Resources.insert({ strId.GetGuid(), createdRes });

		auto result = createdRes->LoadResource();

		if (result == false)
		{
			return nullptr;
		}
		return createdRes;
	}

	Resource* ResourceManager::LoadResource(ResourceHandle handle)
	{
		Resource* res = GetResource({ .id = handle.id });

		if (res != nullptr)
		{
			bool loaded = res->LoadResource();
			if (loaded == true)
			{
				return res;
			}
			return nullptr;
		}
		return nullptr;
	}

	void ResourceManager::UnloadResource(ResourceHandle handle)
	{
		Resource* res = GetResource(handle);
		if (res != nullptr)
		{
			res->UnloadResource();
		}
	}

	void ResourceManager::UnloadResource(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);
		UnloadResource({ .id = strId.GetGuid() });
	}

	void ResourceManager::UnloadAll()
	{
		for (const auto& [key, value] : m_Resources)
		{
			if (value != nullptr)
			{
				value->UnloadResource();
			}
		}
	}

	ResourceHandle ResourceManager::GetHandleForResource(const CzuchStr& path)
	{
		StringID strId = StringID::MakeStringID(path);
		Resource* res = GetResource({ .id = strId.GetGuid() });
		if (res != nullptr)
		{
			return { .id = res->GetGuid() };
		}
		return { .id = InvalidID };
	}

	Resource* ResourceManager::GetResource(ResourceHandle handle)
	{
		auto result = m_Resources.find(handle.id);
		if (result != m_Resources.end())
		{
			return result->second;
		}
		return nullptr;
	}

}
