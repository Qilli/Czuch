#include "czpch.h"
#include "MaterialAsset.h"
#include "./Renderer/GraphicsDevice.h"
#include "./Core/FileHelper.h"
#include "./Subsystems/Assets/AssetsManager.h"

namespace Czuch
{
	MaterialAsset::MaterialAsset(const CzuchStr& path, GraphicsDevice* device, MaterialLoadSettings& loadSettings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path),assetsManager), m_Device(device)
	{
		m_AssetType = AssetType::LOADED_TYPE;
		m_MaterialLoadSettings = std::move(loadSettings);
		LoadAsset();
	}

	MaterialAsset::MaterialAsset(const CzuchStr& path, GraphicsDevice* device, MaterialCreateSettings& settings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path),assetsManager), m_Device(device)
	{
		m_AssetType = AssetType::CREATED_TYPE;
		m_MaterialCreateSettings = std::move(settings);
		CreateFromData();
	}

	MaterialAsset::~MaterialAsset()
	{
		m_ForceUnload = true;
		UnloadAsset();
	}

	bool MaterialAsset::LoadAsset()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			m_RefCounter.Up();
			return true;
		}

		ShaderStage stage = StringToShaderStage(GetTypeFromPath(AssetPath()));

		if (!ReadBinaryFile(AssetPath(), m_MaterialBuffer))
		{
			LOG_BE_ERROR("{0} Failed to load material resource at path {1}", "[MaterialResource]", AssetPath());
			return false;
		}

		//TODO Parse material from file and create material assedt

		if (!HANDLE_IS_VALID(m_MaterialResource))
		{
			return false;
		}
		m_RefCounter.Up();
		m_State = AssetInnerState::LOADED;
		return true;
	}

	bool MaterialAsset::UnloadAsset()
	{
		if (ShouldUnload() && HANDLE_IS_VALID(m_MaterialResource))
		{
			m_Device->Release(m_MaterialResource);
			m_State = AssetInnerState::UNLOADED;
			return true;
		}
		return false;
	}
	bool MaterialAsset::CreateFromData()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			return true;
		}

		m_MaterialResource = m_Device->CreateMaterial(&m_MaterialCreateSettings.desc);
		auto mat = m_Device->AccessMaterial(m_MaterialResource);
		mat->assetHandle = this->GetHandle();
		m_State = AssetInnerState::LOADED;
		return true;
	}

	void MaterialAsset::CopyMaterialDescTo(MaterialDefinitionDesc& desc)
	{
		desc = this->m_MaterialCreateSettings.desc;
	}

}