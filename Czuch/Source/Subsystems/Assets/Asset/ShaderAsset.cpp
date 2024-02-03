#include "czpch.h"
#include "ShaderAsset.h"
#include "./Renderer/GraphicsDevice.h"
#include "./Core/FileHelper.h"

namespace Czuch
{
	ShaderAsset::ShaderAsset(const CzuchStr& path, GraphicsDevice* device): Asset(path,GetNameFromPath(path)),m_Device(device)
	{
		LoadAsset();
	}

	ShaderAsset::~ShaderAsset()
	{
		UnloadAsset();
	}

	bool ShaderAsset::LoadAsset()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			return true;
		}

		ShaderStage stage = StringToShaderStage(GetTypeFromPath(AssetPath()));

		if (!ReadBinaryFile(AssetPath(), m_ShaderCode))
		{
			LOG_BE_ERROR("{0} Failed to load shader resource at path {1}", "[ShaderResource]", AssetPath());
			return false;
		}

		m_ShaderAsset=m_Device->CreateShader(stage, m_ShaderCode.data(), m_ShaderCode.size());

		if (!HANDLE_IS_VALID(m_ShaderAsset))
		{
			return false;
		}

		m_State = AssetInnerState::LOADED;
		return true;
	}

	void ShaderAsset::UnloadAsset()
	{
		if (m_State == AssetInnerState::LOADED && !HANDLE_IS_VALID(m_ShaderAsset))
		{
			m_Device->Release(m_ShaderAsset);
			m_State = AssetInnerState::UNLOADED;
		}
	}
}
