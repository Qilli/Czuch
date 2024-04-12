#pragma once
#include "Component.h"

namespace Czuch
{
	class MeshRendererComponent :
		public Component
	{
	public:
		MeshRendererComponent(Entity owner);
		MeshRendererComponent(Entity owner,MaterialInstanceHandle overrideMaterial);
		~MeshRendererComponent()=default;
		inline MaterialInstanceHandle GetOverrideMaterial(U32 index=0) const { return m_Materials[index]; }
		inline void SetOverrideMaterial(U32 index, MaterialInstanceHandle overrideMaterial) {
			if (index >= m_Materials.size())
			{
				m_Materials.resize(index + 1);
			}
			m_Materials[index] = overrideMaterial;
		}
	private:
		std::vector<MaterialInstanceHandle> m_Materials;
	};
}


