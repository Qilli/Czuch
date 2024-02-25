#include "czpch.h"
#include "ShaderAsset.h"
#include "./Renderer/GraphicsDevice.h"
#include "./Core/FileHelper.h"

namespace Czuch
{
	ShaderAsset::ShaderAsset(const CzuchStr& path, GraphicsDevice* device, ShaderLoadSettings& loadSettings, AssetsManager* assetsManager): Asset(path,GetNameFromPath(path),assetsManager),m_Device(device)
	{
		m_AssetType = AssetType::LOADED_TYPE;
		m_ShaderLoadSettings = std::move(loadSettings);
		LoadAsset();
	}

	ShaderAsset::ShaderAsset(const CzuchStr& path, GraphicsDevice* device, ShaderCreateSettings& settings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path),assetsManager), m_Device(device)
	{
		m_AssetType = AssetType::CREATED_TYPE;
		m_ShaderCreateSettings = std::move(settings);
		CreateFromData();
	}

	ShaderAsset::~ShaderAsset()
	{
		m_ForceUnload = true;
		UnloadAsset();
	}

	bool ShaderAsset::LoadAsset()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			m_RefCounter.Up();
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
		m_RefCounter.Up();
		m_State = AssetInnerState::LOADED;
		return true;
	}

	bool ShaderAsset::UnloadAsset()
	{
		if (m_State == AssetInnerState::LOADED && !HANDLE_IS_VALID(m_ShaderAsset) && (!m_RefCounter.Down()||m_ForceUnload))
		{
			m_Device->Release(m_ShaderAsset);
			m_State = AssetInnerState::UNLOADED;
			return true;
		}
		return false;
	}
	bool ShaderAsset::CreateFromData()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			return true;
		}

		ShaderStage stage = m_ShaderCreateSettings.stage;

		if (m_ShaderCreateSettings.shaderCode.size()<=0)
		{
			LOG_BE_ERROR("{0} Failed to create shader resource from data due to lack of shader code {1}", "[ShaderResource]", AssetPath());
			return false;
		}

		m_ShaderAsset = m_Device->CreateShader(stage, m_ShaderCreateSettings.shaderCode.data(), m_ShaderCreateSettings.shaderCode.size());

		if (!HANDLE_IS_VALID(m_ShaderAsset))
		{
			return false;
		}

		m_State = AssetInnerState::LOADED;
		return true;
		
		return true;
	}
}
