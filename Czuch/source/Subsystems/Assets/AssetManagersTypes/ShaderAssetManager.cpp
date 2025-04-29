#include "czpch.h"
#include "ShaderAssetManager.h"
#include "../BuildInAssets.h"

namespace Czuch
{
	CzuchStr ShaderAssetManager::s_ShaderIncludePath;

	void ShaderAssetManager::Init()
	{
		m_DefaultAsset = GetAsset(DefaultAssets::DEFAULT_SIMPLE_MATERIAL_ASSET);
	}
}
