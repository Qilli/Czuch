#pragma once
#include "ResourceManager.h"
#include "ShaderResource.h"

namespace Czuch
{
	class ShaderResourceManager :public ResourceManager
	{
	public:
		ShaderResourceManager(GraphicsDevice* device);
	protected:
		Resource* CreateResource(const CzuchStr& path) override;
	private:
		GraphicsDevice* m_Device;
	};

	ShaderResourceManager::ShaderResourceManager(GraphicsDevice* device):m_Device(device)
	{
	}

	Resource* ShaderResourceManager::CreateResource(const CzuchStr& path)
	{
		ShaderResource* shaderRes = new ShaderResource(path,m_Device);
		return shaderRes;
	}

}

