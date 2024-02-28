#pragma once
#include"../AssetsManager.h"
#include "../Asset/ModelAsset.h"

namespace Czuch
{
    class ModelAssetManager :public AssetManager
	{
	public:
		ModelAssetManager(GraphicsDevice* device);
	protected:
		Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) override;
		Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) override;
	private:
		GraphicsDevice* m_Device;
	};

	inline ModelAssetManager::ModelAssetManager(GraphicsDevice* device) :m_Device(device)
	{
	}

	inline Asset* ModelAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		ModelAsset* modelRes = new ModelAsset(path,(ModelCreateSettings&)settings, m_Device,AssetsManager::GetPtr());
		StringID strId = StringID::MakeStringID(path);
		RegisterAsset(strId, modelRes);
		return modelRes;
	}

	inline Asset* ModelAssetManager::CreateAsset(const CzuchStr& path, BaseLoadSettings& settings)
	{
		ModelAsset* modelRes = new ModelAsset(path, (ModelLoadSettings&)settings, m_Device,AssetsManager::GetPtr());
		return modelRes;
	}

}

