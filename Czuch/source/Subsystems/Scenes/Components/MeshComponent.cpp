#include "czpch.h"
#include "MeshComponent.h"
#include "Subsystems/Assets/AssetsManager.h"
#include "Subsystems/Assets/Asset/ModelAsset.h"
namespace Czuch
{
	MeshComponent::MeshComponent(Entity owner): Component::Component(owner)
	{

	}

	MeshComponent::MeshComponent(Entity owner, AssetHandle model,MeshHandle mesh): Component::Component(owner)
	{
		SetMesh(model, mesh);
	}

	void MeshComponent::DecrementMeshReference()
	{
		if (HANDLE_IS_VALID(m_Mesh))
		{
			AssetsManager::GetPtr()->UnloadAsset<ModelAsset>(m_Model);
		}
	}

	void MeshComponent::IncrementMeshReference()
	{
		if (HANDLE_IS_VALID(m_Mesh))
		{
			AssetsManager::GetPtr()->IncrementAssetRef<ModelAsset>(m_Model);
		}
	}

}
