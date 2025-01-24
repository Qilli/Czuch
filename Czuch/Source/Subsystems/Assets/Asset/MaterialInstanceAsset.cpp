#include "czpch.h"
#include "MaterialInstanceAsset.h"
#include "./Renderer/GraphicsDevice.h"
#include "./Core/FileHelper.h"
#include "./Subsystems/Assets/AssetsManager.h"
#include "./Subsystems/Assets/Asset/TextureAsset.h"

namespace Czuch
{
	MaterialInstanceAsset::MaterialInstanceAsset(const CzuchStr& path, GraphicsDevice* device, MaterialInstanceLoadSettings& loadSettings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path),assetsManager), m_Device(device)
	{
		m_AssetType = AssetModeType::LOADED_TYPE;
		m_MaterialLoadSettings = std::move(loadSettings);
	}

	MaterialInstanceAsset::MaterialInstanceAsset(const CzuchStr& path, GraphicsDevice* device, MaterialInstanceCreateSettings& settings, AssetsManager* assetsManager) : Asset(path, GetNameFromPath(path),assetsManager), m_Device(device)
	{
		m_AssetType = AssetModeType::CREATED_TYPE;
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
		if (Asset::LoadAsset())
		{
			return true;
		}

		if (m_AssetType == AssetModeType::CREATED_TYPE)
		{
			LoadDependencies();
			m_MaterialInstanceResource = m_Device->CreateMaterialInstance(m_MaterialInstanceDesc);
			m_State = AssetInnerState::LOADED;
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
			UnloadDependencies();
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

		
		m_MaterialInstanceDesc = std::move(m_MaterialCreateSettings.desc);
		m_MaterialInstanceDesc.GetAllTexturesDependencies(m_Dependencies);
		LoadDependencies();
		m_MaterialInstanceResource = m_Device->CreateMaterialInstance(m_MaterialInstanceDesc);
		m_State = AssetInnerState::LOADED;
		m_RefCounter.Up();
		return true;
	}
	ShortAssetInfo* MaterialInstanceAsset::GetShortAssetInfo()
	{
		if (m_ShortInfo.name == nullptr || m_ShortInfo.name->empty())
		{
			m_ShortInfo.name = &AssetName();
			m_ShortInfo.type = AssetType::MATERIAL;
			m_ShortInfo.asset = GetHandle();
			m_ShortInfo.resource = m_MaterialInstanceResource.handle;
		}

		return &m_ShortInfo;
	}
	CzuchStr MaterialInstanceAsset::GetAssetLoadInfo() const
	{
		return "MaterialInstanceAsset: " + AssetName()+" Ref count: "+m_RefCounter.GetCountString() + " State: " + (m_State == AssetInnerState::LOADED ? " Loaded" : "Unloaded");
	}
	void MaterialInstanceAsset::LoadDependencies()
	{
		for (auto& dep : m_Dependencies)
		{
			auto asset = AssetsManager::GetPtr()->GetAsset<TextureAsset>(dep.assetHandle,true);
			if (asset==nullptr || !asset->IsLoaded())
			{
				LOG_BE_ERROR("{0} Failed to load texture dependency: {1}", "[MaterialInstanceAsset]", dep.assetHandle.handle);
			}
		}
	}
	void MaterialInstanceAsset::UnloadDependencies()
	{
		for (auto& dep : m_Dependencies)
		{
			AssetsManager::GetPtr()->UnloadAsset<TextureAsset>(dep.assetHandle);
		}
	}
}
