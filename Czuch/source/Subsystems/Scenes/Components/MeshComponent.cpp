#include "czpch.h"
#include "MeshComponent.h"
#include "Subsystems/Assets/AssetsManager.h"
#include "Subsystems/Assets/Asset/ModelAsset.h"
#include "TransformComponent.h"
namespace Czuch
{
	MeshComponent::MeshComponent(Entity owner): Component::Component(owner)
	{

	}

	MeshComponent::MeshComponent(Entity owner, AssetHandle model,MeshHandle mesh): Component::Component(owner)
	{
		SetMesh(model, mesh);
	}

	OBB MeshComponent::GetOBB(TransformComponent& transform,float scale) const
	{
		return TransformAABBToOBB(GetAABB(scale), transform.GetLocalToWorld());
	}

	AABB MeshComponent::GetAABB(float scale) const
	{
		auto* modelAsset=AssetsManager::GetPtr()->GetAsset<ModelAsset>(m_Model);
		if (modelAsset)
		{
			auto* mesh = modelAsset->GetMeshData(m_Mesh);
			if (mesh)
			{
				return mesh->aabb.GetScaled(scale);
			}
		}
		return AABB();
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
