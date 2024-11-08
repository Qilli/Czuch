#include "czpch.h"
#include "ModelAsset.h"
#include "./Renderer/GraphicsDevice.h"
#include"../AssetsManager.h"
#include"MaterialAsset.h"
#include"MaterialInstanceAsset.h"

namespace Czuch
{
	ModelAsset::ModelAsset(const CzuchStr& path, ModelLoadSettings& loadSettings, GraphicsDevice* device, AssetsManager* assetsManager):Asset(path, GetNameFromPath(path),assetsManager),m_Device(device)
	{
		m_AssetType = AssetType::LOADED_TYPE;
		m_CurrentLoadSettings = std::move(loadSettings);
		LoadAsset();
	}

	ModelAsset::ModelAsset(const CzuchStr& path, ModelCreateSettings& settings, GraphicsDevice* device, AssetsManager* assetsManager) :Asset(path,path,assetsManager), m_Device(device)
	{
		m_AssetType = AssetType::CREATED_TYPE;
		m_CreateSettings = std::move(settings);
		CreateFromData();
	}

	ModelAsset::~ModelAsset()
	{
		m_ForceUnload = true;
		UnloadAsset();
	}

	bool ModelAsset::LoadAsset()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			m_RefCounter.Up();
			return true;
		}

		if (m_AssetType == AssetType::CREATED_TYPE)
		{
			return false;
		}

		//todo add loader

		return true;
	}

	bool ModelAsset::UnloadAsset()
	{
		if (ShouldUnload())
		{
			for (auto m : m_Meshes)
			{
				m_Device->Release(m);
			}

			for (auto m : m_Materials)
			{
				m_AssetsMgr->UnloadAsset<MaterialInstanceAsset>(m);
			}

			m_Meshes.clear();
			m_Materials.clear();
			m_State = AssetInnerState::UNLOADED;
			return true;
		}
		return false;
	}

	bool ModelAsset::CreateFromData()
	{
		if (m_State == AssetInnerState::LOADED)
		{
			return true;
		}

		if (m_AssetType == AssetType::LOADED_TYPE)
		{
			return false;
		}

		for (auto &meshData : m_CreateSettings.meshesData)
		{
			auto meshHandle= m_Device->CreateMesh(meshData);
			Mesh* mesh = m_Device->AccessMesh(meshHandle);
			MaterialInstance* instance = m_Device->AccessMaterialInstance(meshData.material);
			Material* material = m_Device->AccessMaterial(instance->handle);
			m_Meshes.push_back(meshHandle);
			m_Materials.push_back(material->assetHandle);
		}

		m_State = AssetInnerState::LOADED;

		return true;
	}
}
