#pragma once
#include "Component.h"

namespace Czuch
{
	class CZUCH_API MeshRendererComponent :
		public Component
	{
	public:
		MeshRendererComponent(Entity owner);
		MeshRendererComponent(Entity owner, AssetHandle overrideMaterial);
		~MeshRendererComponent()=default;
		inline MaterialInstanceHandle GetOverrideMaterial() const { return m_Material; }
		void SetOverrideMaterial(AssetHandle overrideMaterial);
	private:
		bool HasMaterial() const { return HANDLE_IS_VALID(m_Material); }
		MaterialInstanceHandle m_Material;
		AssetHandle m_MaterialAsset;
	};
}


