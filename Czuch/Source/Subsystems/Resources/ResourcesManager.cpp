#include "czpch.h"
#include "ResourcesManager.h"
#include <typeinfo>

namespace Czuch
{

	void ResourcesManager::Init()
	{

	}

	void ResourcesManager::Shutdown()
	{
		for (const auto& [key,value]: m_ResourcesMgrs)
		{
			delete value;
		}
		m_ResourcesMgrs.clear();
	}

	void ResourcesManager::Update()
	{
	}

	void ResourcesManager::RegisterManager(ResourceManager* newMgr, std::type_index type)
	{
		m_ResourcesMgrs.insert({type,newMgr});
	}

}
