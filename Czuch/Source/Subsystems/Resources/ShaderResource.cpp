#include "czpch.h"
#include "ShaderResource.h"
#include "./Renderer/GraphicsDevice.h"
#include "./Core/FileHelper.h"

namespace Czuch
{
	ShaderResource::ShaderResource(const CzuchStr& path, GraphicsDevice* device): Resource(path,GetNameFromPath(path)),m_Device(device)
	{
		LoadResource();
	}

	ShaderResource::~ShaderResource()
	{
		UnloadResource();
	}

	bool ShaderResource::LoadResource()
	{
		if (m_State == ResourceInnerState::LOADED)
		{
			return true;
		}

		ShaderStage stage = StringToShaderStage(GetTypeFromPath(ResourcePath()));

		if (!ReadBinaryFile(ResourcePath(), m_ShaderCode))
		{
			LOG_BE_ERROR("{0} Failed to load shader resource at path {1}", "[ShaderResource]", ResourcePath());
			return false;
		}

		m_ShaderAsset=m_Device->CreateShader(stage, m_ShaderCode.data(), m_ShaderCode.size());

		if (m_ShaderAsset == nullptr)
		{
			return false;
		}

		m_State = ResourceInnerState::LOADED;
		return true;
	}

	void ShaderResource::UnloadResource()
	{
		if (m_State == ResourceInnerState::LOADED && m_ShaderAsset != nullptr)
		{
			m_Device->ReleaseShader(m_ShaderAsset);
			m_State = ResourceInnerState::UNLOADED;
		}
	}
}
