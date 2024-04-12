#include "czpch.h"
#include "Entity.h"
#include"Components/TransformComponent.h"
#include"Components/MeshRendererComponent.h"
#include"Components/MeshComponent.h"

namespace Czuch
{
	Entity::Entity(entt::entity handle, IScene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	void Entity::AddRenderable(MeshHandle meshHandle, MaterialInstanceHandle overrideMaterial)
	{
		AddComponent<MeshComponent>(*this, meshHandle);
		AddComponent<MeshRendererComponent>(*this, overrideMaterial);
	}

	void Entity::RemoveRenderable()
	{
		RemoveComponent<MeshComponent>();
		RemoveComponent<MeshRendererComponent>();
	}

	bool Entity::IsRenderable()
	{
		return HasComponent<MeshComponent>() && HasComponent<MeshRendererComponent>();
	}

	void Entity::AddChild(Entity& entity)
	{
		GetComponent<TransformComponent>().AddChild(entity);
	}

	void Entity::SetAsParent(Entity& entity)
	{
		GetComponent<TransformComponent>().SetParent(entity);
	}

	bool Entity::operator==(const Entity& other) const
	{
		return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
	}

	void Entity::SetActive(bool isActive)
	{
		bool hasActiveComponent = HasComponent<ActiveComponent>();
		if (hasActiveComponent&&!isActive)
		{
			RemoveComponent<ActiveComponent>();
		}
		else if (!hasActiveComponent && isActive)
		{
			AddComponent<ActiveComponent>(*this);
		}
	}

	TransformComponent& Entity::Transform() { return GetComponent<TransformComponent>(); }

}

