#include "czpch.h"
#include "MaterialInstanceAsset.h"
#include "./Renderer/GraphicsDevice.h"
#include "./Core/FileHelper.h"
#include "./Subsystems/Assets/AssetsManager.h"

namespace Czuch
{
	MaterialInstanceAsset::MaterialInstanceAsset(const CzuchStr& path, GraphicsDevice* device, MaterialInstanceLoadSettings& loadSettings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path),assetsManager), m_Device(device)
	{
		m_AssetType = AssetType::LOADED_TYPE;
		m_MaterialLoadSettings = std::move(loadSettings);
		LoadAsset();
	}

	MaterialInstanceAsset::MaterialInstanceAsset(const CzuchStr& path, GraphicsDevice* device, MaterialInstanceCreateSettings& settings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path),assetsManager), m_Device(device)
	{
		m_AssetType = AssetType::CREATED_TYPE;
		m_MaterialCreateSettings = std::move(settings);
		CreateFromData();
	}

	MaterialInstanceAsset::~MaterialInstanceAsset()
	{
		m_ForceUnload = true;
		UnloadAsset();
	}

	bool MaterialInstanceAsset::LoadAsset()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			m_RefCounter.Up();
			return true;
		}

		if (!ReadBinaryFile(AssetPath(), m_MaterialInstanceBuffer))
		{
			LOG_BE_ERROR("{0} Failed to load material instance resource at path {1}", "[MaterialInstanceResource]", AssetPath());
			return false;
		}

		//TODO Parse material from file and create material assedt

		if (!HANDLE_IS_VALID(m_MaterialInstanceResource))
		{
			return false;
		}
		m_RefCounter.Up();
		m_State = AssetInnerState::LOADED;
		return true;
	}

	bool MaterialInstanceAsset::UnloadAsset()
	{
		if (ShouldUnload() && HANDLE_IS_VALID(m_MaterialInstanceResource))
		{
			m_Device->Release(m_MaterialInstanceResource);
			m_State = AssetInnerState::UNLOADED;
			return true;
		}
		return false;
	}
	bool MaterialInstanceAsset::CreateFromData()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			return true;
		}

		m_MaterialInstanceResource = m_Device->CreateMaterialInstance(m_MaterialCreateSettings.desc);
		m_State = AssetInnerState::LOADED;
		return true;
	}
}
