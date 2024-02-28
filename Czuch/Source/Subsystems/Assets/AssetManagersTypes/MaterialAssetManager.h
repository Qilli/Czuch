#pragma once
#include"../AssetsManager.h"
#include "../Asset/MaterialAsset.h"

namespace Czuch
{
    class MaterialAssetManager :public AssetManager
	{
	public:
		MaterialAssetManager(GraphicsDevice* device);
	protected:
		Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) override;
		Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) override;
	private:
		GraphicsDevice* m_Device;
	};

	inline MaterialAssetManager::MaterialAssetManager(GraphicsDevice* device) :m_Device(device)
	{
	}

	inline Asset* MaterialAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		MaterialAsset* matRes = new MaterialAsset(path,m_Device,(MaterialCreateSettings&)settings,AssetsManager::GetPtr());
		StringID strId = StringID::MakeStringID(path);
		RegisterAsset(strId, matRes);
		return matRes;
	}

	inline Asset* MaterialAssetManager::CreateAsset(const CzuchStr& path, BaseLoadSettings& settings)
	{
		MaterialAsset* matRes = new MaterialAsset(path,m_Device, (MaterialLoadSettings&)settings,AssetsManager::GetPtr());
		return matRes;
	}

}

