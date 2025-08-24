#include "czpch.h"
#include "TextureAssetManager.h"
#include "../BuildInAssets.h"
#include "Renderer/GraphicsDevice.h"
namespace Czuch
{

	void TextureAssetManager::Init()
	{
		m_DefaultAsset = GetAsset(DefaultAssets::PINK_TEXTURE_ASSET);
	}

	Asset* TextureAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		TextureAsset* texRes = new TextureAsset(path, (TextureCreateSettings&)settings, m_Device, AssetsManager::GetPtr());
		StringID strId = StringID::MakeStringID(path);
		RegisterAsset(strId, texRes);
		return texRes;
	}

}
