#pragma once
#include"../AssetsManager.h"
#include "../Asset/MaterialInstanceAsset.h"

namespace Czuch
{
    class MaterialInstanceAssetManager :public AssetManager
	{
	public:
		MaterialInstanceAssetManager(GraphicsDevice* device);
		bool IsFormatSupported(const char* format)const override { return strcmp(format, ".mat") == 0; }
		void Init() override;
		int GetAssetType() const override { return (int)AssetType::MATERIAL_INSTANCE; }
	protected:
		Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) override;
		Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) override;
		Asset* CreateLoadableAsset(const CzuchStr& path) override;
	private:
		GraphicsDevice* m_Device;
	};

	inline MaterialInstanceAssetManager::MaterialInstanceAssetManager(GraphicsDevice* device) :m_Device(device)
	{
	}

	inline Asset* MaterialInstanceAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		StringID strId = StringID::MakeStringID(path);
		MaterialInstanceAsset* matRes = new MaterialInstanceAsset(path,m_Device,(MaterialInstanceCreateSettings&)settings,AssetsManager::GetPtr());
		RegisterAsset(strId, matRes);
		return matRes;
	}

	inline Asset* MaterialInstanceAssetManager::CreateAsset(const CzuchStr& path, BaseLoadSettings& settings)
	{
		MaterialInstanceAsset* matRes = new MaterialInstanceAsset(path,m_Device, (MaterialInstanceLoadSettings&)settings,AssetsManager::GetPtr());
		return matRes;
	}

	inline Asset* MaterialInstanceAssetManager::CreateLoadableAsset(const CzuchStr& path)
	{
		MaterialInstanceLoadSettings settings;
		MaterialInstanceAsset* matRes = new MaterialInstanceAsset(path, m_Device, settings, AssetsManager::GetPtr());
		return matRes;
	}

}

