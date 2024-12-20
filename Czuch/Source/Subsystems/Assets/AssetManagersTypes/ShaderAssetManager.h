#pragma once
#include"../AssetsManager.h"
#include "../Asset/ShaderAsset.h"

namespace Czuch
{
	class ShaderAssetManager :public AssetManager
	{
	public:
		ShaderAssetManager(GraphicsDevice* device);
	protected:
		Asset* CreateAsset(const CzuchStr& name, BaseCreateSettings& settings) override;
		Asset* CreateAsset(const CzuchStr& path, BaseLoadSettings& settings) override;
	private:
		GraphicsDevice* m_Device;
	};

	inline ShaderAssetManager::ShaderAssetManager(GraphicsDevice* device):m_Device(device)
	{
	}

	inline Asset* ShaderAssetManager::CreateAsset(const CzuchStr& path, BaseCreateSettings& settings)
	{
		ShaderAsset* shaderRes = new ShaderAsset(path,m_Device,(ShaderCreateSettings&)settings,AssetsManager::GetPtr());
		StringID strId = StringID::MakeStringID(path);
		RegisterAsset(strId, shaderRes);
		return shaderRes;
	}

	inline Asset* ShaderAssetManager::CreateAsset(const CzuchStr& path, BaseLoadSettings& settings)
	{
		ShaderAsset* shaderRes = new ShaderAsset(path, m_Device,(ShaderLoadSettings&)settings,AssetsManager::GetPtr());
		return shaderRes;
	}

}

