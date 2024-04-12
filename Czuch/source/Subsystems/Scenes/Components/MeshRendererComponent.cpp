#include "czpch.h"
#include "MeshRendererComponent.h"

namespace Czuch
{
	Czuch::MeshRendererComponent::MeshRendererComponent(Entity owner) : Component(owner)
	{
	}

	MeshRendererComponent::MeshRendererComponent(Entity owner, MaterialInstanceHandle overrideMaterial): Component(owner)
	{
		m_Materials.clear();
		SetOverrideMaterial(0, overrideMaterial);
	}

}
