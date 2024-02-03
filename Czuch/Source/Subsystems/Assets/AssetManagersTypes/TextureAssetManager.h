#pragma once
#include "../AssetManager.h"
#include "../Asset/TextureAsset.h"

namespace Czuch
{
    class TextureAssetManager :public AssetManager
	{
	public:
		TextureAssetManager(GraphicsDevice* device);
	protected:
		Asset* CreateAsset(const CzuchStr& path, void* settings) override;
	private:
		GraphicsDevice* m_Device;
	};

	inline TextureAssetManager::TextureAssetManager(GraphicsDevice* device) :m_Device(device)
	{
	}

	inline Asset* TextureAssetManager::CreateAsset(const CzuchStr& path, void* settings)
	{
		TextureLoadSettings* set = (TextureLoadSettings*)settings;
		TextureAsset* shaderRes = new TextureAsset(path,set, m_Device);
		return shaderRes;
	}

}

