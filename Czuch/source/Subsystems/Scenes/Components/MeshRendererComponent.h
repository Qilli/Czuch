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
		AssetHandle GetMaterialAsset() const { return m_MaterialAsset; }

		inline void SetShadowCasting(bool castShadow) { m_CastShadow = castShadow; }
		inline bool IsShadowCaster() const { return m_CastShadow; }

		inline void SetReceiveShadow(bool receiveShadow) { m_ReceiveShadow = receiveShadow; }
		inline bool IsReceivingShadow() const { return m_ReceiveShadow; }

		void OnRemoved() override {
			if (HasMaterial())
			{
				DecrementDependenciesReference();
			}
		}
	private:
		bool HasMaterial() const { return HANDLE_IS_VALID(m_Material); }
		void DecrementDependenciesReference();
		MaterialInstanceHandle m_Material;
		AssetHandle m_MaterialAsset;
		bool m_CastShadow;
		bool m_ReceiveShadow;
	};
}


