#include "czpch.h"
#include "TextureAssetManager.h"

namespace Czuch
{

	Asset* TextureAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		TextureAsset* texRes = new TextureAsset(path, (TextureCreateSettings&)settings, m_Device, AssetsManager::GetPtr());
		StringID strId = StringID::MakeStringID(path);
		RegisterAsset(strId, texRes);
		return texRes;
	}
}
