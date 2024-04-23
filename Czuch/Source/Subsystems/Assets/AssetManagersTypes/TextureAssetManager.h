#pragma once
#include"../AssetsManager.h"
#include "../Asset/TextureAsset.h"

namespace Czuch
{
    class TextureAssetManager :public AssetManager
	{
	public:
		TextureAssetManager(GraphicsDevice* device);
	protected:
		Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) override;
		Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) override;
	private:
		GraphicsDevice* m_Device;
	};

	inline TextureAssetManager::TextureAssetManager(GraphicsDevice* device) :m_Device(device)
	{
	}

	inline Asset* TextureAssetManager::CreateAsset(const CzuchStr& path, BaseLoadSettings& settings)
	{
		TextureAsset* texRes = new TextureAsset(path, (TextureLoadSettings&)settings, m_Device,AssetsManager::GetPtr());
		return texRes;
	}

}

