#pragma once
#include"../AssetsManager.h"
#include "../Asset/MaterialAsset.h"

namespace Czuch
{
    class MaterialAssetManager :public AssetManager
	{
	public:
		MaterialAssetManager(GraphicsDevice* device);
		bool IsFormatSupported(const char* format)const override { return strcmp(format, ".matdef") == 0; }
		int GetAssetType() const override { return (int)AssetType::MATERIAL; }
	public:
		void Init() override;
	protected:
		Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) override;
		Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) override;
		Asset* CreateLoadableAsset(const CzuchStr& path) override;
	private:
		GraphicsDevice* m_Device;
	};

	inline MaterialAssetManager::MaterialAssetManager(GraphicsDevice* device) :m_Device(device)
	{
	}
	

	inline Asset* MaterialAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		StringID strId = StringID::MakeStringID(path);
		MaterialAsset* matRes = new MaterialAsset(path,m_Device,(MaterialCreateSettings&)settings,AssetsManager::GetPtr());
		RegisterAsset(strId, matRes);
		return matRes;
	}

	inline Asset* MaterialAssetManager::CreateAsset(const CzuchStr& path, BaseLoadSettings& settings)
	{
		MaterialAsset* matRes = new MaterialAsset(path,m_Device, (MaterialLoadSettings&)settings,AssetsManager::GetPtr());
		return matRes;
	}

	inline Asset* MaterialAssetManager::CreateLoadableAsset(const CzuchStr& path)
	{
		MaterialLoadSettings settings;
		MaterialAsset* matRes = new MaterialAsset(path, m_Device, settings, AssetsManager::GetPtr());
		return matRes;
	}

}

