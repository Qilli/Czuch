#pragma once
#include "../AssetManager.h"
#include "../Asset/ShaderAsset.h"

namespace Czuch
{
	class ShaderAssetManager :public AssetManager
	{
	public:
		ShaderAssetManager(GraphicsDevice* device);
	protected:
		Asset* CreateAsset(const CzuchStr& path,void* settings) override;
	private:
		GraphicsDevice* m_Device;
	};

	inline ShaderAssetManager::ShaderAssetManager(GraphicsDevice* device):m_Device(device)
	{
	}

	inline Asset* ShaderAssetManager::CreateAsset(const CzuchStr& path,void* settings)
	{
		ShaderAsset* shaderRes = new ShaderAsset(path,m_Device);
		return shaderRes;
	}

}

