#include "czpch.h"
#include "ModelAssetManager.h"
#include "../BuildInAssets.h"

namespace Czuch
{
	void ModelAssetManager::Init()
	{
		m_DefaultAsset = GetAsset(DefaultAssets::CUBE_ASSET);
	}
}
