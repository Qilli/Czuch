#include "czpch.h"
#include "MeshComponent.h"

namespace Czuch
{
	MeshComponent::MeshComponent(Entity owner): Component::Component(owner)
	{

	}

	MeshComponent::MeshComponent(Entity owner, MeshHandle mesh): Component::Component(owner)
	{
		m_SubMeshes.push_back(mesh);
	}

}
