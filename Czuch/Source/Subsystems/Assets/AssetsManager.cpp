#include "czpch.h"
#include "AssetsManager.h"
#include <typeinfo>
#include "./Subsystems/Logging.h"

namespace Czuch
{

	void AssetsManager::Init()
	{

	}

	void AssetsManager::Shutdown()
	{
		for (const auto& [key, value] : m_AssetsMgrs)
		{
			delete value;
		}
		m_AssetsMgrs.clear();
	}

	void AssetsManager::Update()
	{
	}

	void AssetsManager::RegisterManager(AssetManager* newMgr, std::type_index type)
	{
		m_AssetsMgrs.insert({type,newMgr});
	}

}
