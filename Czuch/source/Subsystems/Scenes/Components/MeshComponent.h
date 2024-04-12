#pragma once
#include "Component.h"

namespace Czuch
{
	class MeshComponent :public Component
	{
	public:
		MeshComponent(Entity owner);
		MeshComponent(Entity owner,MeshHandle mesh);
		~MeshComponent() = default;
		bool HasMesh() const { return m_SubMeshes.size()>0; }
		MeshHandle GetMesh(U32 index=0) const { return m_SubMeshes[index]; }
		void AddMesh(MeshHandle mesh) { m_SubMeshes.push_back(mesh); }
		U32 GetSubMeshesCount() const { return m_SubMeshes.size(); }
	private:
		std::vector<MeshHandle> m_SubMeshes;
	};
}

