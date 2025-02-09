#pragma once
#include "Component.h"

namespace Czuch
{
	struct CZUCH_API MeshComponent :public Component
	{
	public:
		MeshComponent(Entity owner);
		MeshComponent(Entity owner,AssetHandle model,MeshHandle mesh);
		~MeshComponent()=default;
		bool HasMesh() const { return HANDLE_IS_VALID(m_Mesh); }
		MeshHandle GetMesh() const { return m_Mesh; }
		AssetHandle GetModel() const { return m_Model; }
		void SetMesh(AssetHandle model, MeshHandle mesh) { 

			if (mesh.handle != m_Mesh.handle)
			{
				if (HasMesh())
				{
					DecrementMeshReference();
				}
				SetNewMesh(model,mesh);
			}
		}
		void OnRemoved() override {
			if (HasMesh())
			{
				DecrementMeshReference();
			}
		}

	private:
		void SetNewMesh(AssetHandle model, MeshHandle mesh)
		{
			m_Mesh = mesh;
			m_Model = model;
			IncrementMeshReference();
		}
		void DecrementMeshReference();
		void IncrementMeshReference();
	private:
		MeshHandle m_Mesh;
		AssetHandle m_Model;
	};
}

