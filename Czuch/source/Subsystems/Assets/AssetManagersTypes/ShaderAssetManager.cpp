#include "czpch.h"
#include "ShaderAssetManager.h"
#include "../BuildInAssets.h"

namespace Czuch
{
	void ShaderAssetManager::Init()
	{
		m_DefaultAsset = GetAsset(DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET);
	}
}
