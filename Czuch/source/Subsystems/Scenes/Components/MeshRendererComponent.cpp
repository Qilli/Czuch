#include "czpch.h"
#include "MeshRendererComponent.h"
#include "Subsystems/Assets/AssetsManager.h"
#include "Subsystems/Assets/Asset/MaterialInstanceAsset.h"

namespace Czuch
{
	Czuch::MeshRendererComponent::MeshRendererComponent(Entity owner) : Component(owner)
	{
		SetOverrideMaterial(DefaultAssets::DEFAULT_SIMPLE_MATERIAL_INSTANCE_ASSET);
	}

	MeshRendererComponent::MeshRendererComponent(Entity owner,AssetHandle overrideMaterial): Component(owner)
	{
		SetOverrideMaterial(overrideMaterial);
	}

	void MeshRendererComponent::SetOverrideMaterial(AssetHandle overrideMaterial)
	{
		if (HasMaterial())
		{
			if (m_MaterialAsset == overrideMaterial)
			{
				return;
			}

			AssetsManager::GetPtr()->UnloadAsset<MaterialInstanceAsset>(m_MaterialAsset);
		}

		m_MaterialAsset = overrideMaterial;
		auto mat = AssetsManager::GetPtr()->GetAsset<MaterialInstanceAsset>(overrideMaterial);
		m_Material = mat->GetMaterialInstanceResourceHandle();
		AssetsManager::GetPtr()->IncrementAssetRef<MaterialInstanceAsset>(overrideMaterial);
	}

	void MeshRendererComponent::DecrementDependenciesReference()
	{
		AssetsManager::GetPtr()->UnloadAsset<MaterialInstanceAsset>(m_MaterialAsset);
	}

}
