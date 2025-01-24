#include "czpch.h"
#include "MaterialInstanceAssetManager.h"
#include "../BuildInAssets.h"

namespace Czuch
{
	void MaterialInstanceAssetManager::Init()
	{
		m_DefaultAsset = GetAsset(DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET);
	}
}
