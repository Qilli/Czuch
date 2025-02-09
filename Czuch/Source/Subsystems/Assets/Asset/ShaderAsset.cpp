#include "czpch.h"
#include "ShaderAsset.h"
#include "./Renderer/GraphicsDevice.h"
#include "./Core/FileHelper.h"

namespace Czuch
{
	ShaderAsset::ShaderAsset(const CzuchStr& path, GraphicsDevice* device, ShaderLoadSettings& loadSettings, AssetsManager* assetsManager): Asset(path,GetNameFromPath(path),assetsManager),m_Device(device)
	{
		m_AssetType = AssetModeType::LOADED_TYPE;
		m_ShaderLoadSettings = std::move(loadSettings);
	}

	ShaderAsset::ShaderAsset(const CzuchStr& path, GraphicsDevice* device, ShaderCreateSettings& settings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path),assetsManager), m_Device(device)
	{
		m_AssetType = AssetModeType::CREATED_TYPE;
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
		if (Asset::LoadAsset())
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
		else
		{
			LOG_BE_INFO("Loaded new shader with path: {0}", AssetPath());
		}
		m_RefCounter.Up();
		m_State = AssetInnerState::LOADED;
		return true;
	}

	bool ShaderAsset::UnloadAsset()
	{
		if (ShouldUnload() && HANDLE_IS_VALID(m_ShaderAsset))
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
		m_RefCounter.Up();
		return true;
	}
	CzuchStr ShaderAsset::GetAssetLoadInfo() const
	{
		return "ShaderAsset: " + AssetName() + " Ref count: " + m_RefCounter.GetCountString() + " State: " + (m_State == AssetInnerState::LOADED ? " Loaded" : "Unloaded");
	}

	ShortAssetInfo* ShaderAsset::GetShortAssetInfo()
	{
		if (m_ShortInfo.name == nullptr || m_ShortInfo.name->empty())
		{
			m_ShortInfo.name = &AssetName();
			m_ShortInfo.type = AssetType::SHADER;
			m_ShortInfo.asset = GetHandle();
		}

		m_ShortInfo.resource = m_ShaderAsset.handle;

		return &m_ShortInfo;
	}
}
