#pragma once
#include"../AssetsManager.h"
#include "../Asset/ModelAsset.h"

namespace Czuch
{
    class ModelAssetManager :public AssetManager
	{
	public:
		ModelAssetManager(GraphicsDevice* device);
		bool IsFormatSupported(const char* format)const override { return strcmp(format, ".obj") == 0 || strcmp(format, ".gltf") == 0; }
		void Init() override;
		int GetAssetType() const override { return (int)AssetType::MESH; }
	protected:
		Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) override;
		Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) override;
		Asset* CreateLoadableAsset(const CzuchStr& path) override;
	private:
		GraphicsDevice* m_Device;
	};

	inline ModelAssetManager::ModelAssetManager(GraphicsDevice* device) :m_Device(device)
	{
	}

	inline Asset* ModelAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		StringID strId = StringID::MakeStringID(path);
		ModelAsset* modelRes = new ModelAsset(path,(ModelCreateSettings&)settings, m_Device,AssetsManager::GetPtr());
		RegisterAsset(strId, modelRes);
		return modelRes;
	}

	inline Asset* ModelAssetManager::CreateAsset(const CzuchStr& path, BaseLoadSettings& settings)
	{
		ModelAsset* modelRes = new ModelAsset(path, (ModelLoadSettings&)settings, m_Device,AssetsManager::GetPtr());
		return modelRes;
	}

	inline Asset* ModelAssetManager::CreateLoadableAsset(const CzuchStr& path)
	{
		ModelLoadSettings settings;
		ModelAsset* modelRes = new ModelAsset(path, settings, m_Device, AssetsManager::GetPtr());
		return modelRes;
	}

}

