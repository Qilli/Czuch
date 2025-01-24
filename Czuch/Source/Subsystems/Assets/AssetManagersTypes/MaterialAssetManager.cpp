#include "czpch.h"
#include "MaterialAssetManager.h"
#include "../BuildInAssets.h"

namespace Czuch
{
	void MaterialAssetManager::Init()
	{
		m_DefaultAsset = GetAsset(DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET);
	}
}
