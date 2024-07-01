#include "czpch.h"
#include "Entity.h"
#include"Components/TransformComponent.h"
#include"Components/MeshRendererComponent.h"
#include"Components/MeshComponent.h"
#include"Components/HeaderComponent.h"
#include"Serialization/SerializationComponentHelper.h"

namespace Czuch
{
	Entity::Entity(entt::entity handle, IScene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	void Entity::AddRenderable(MeshHandle meshHandle, MaterialInstanceHandle overrideMaterial)
	{
		AddComponent<MeshComponent>(meshHandle);
		AddComponent<MeshRendererComponent>(overrideMaterial);
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
		if (hasActiveComponent && !isActive)
		{
			RemoveComponent<ActiveComponent>();
		}
		else if (!hasActiveComponent && isActive)
		{
			AddComponent<ActiveComponent>();
		}
	}

	TransformComponent& Entity::Transform() { return GetComponent<TransformComponent>(); }


#pragma region Serialization
	bool Entity::Serialize(YAML::Emitter& out, bool binary)
	{
		if (binary)
		{
			return false;
		}
		else
		{

			SerializerHelper::SetEmitter(&out);
			SerializerHelper::BeginMap();
			SerializerHelper::Key("Entity");
			SerializerHelper::Value("GUID:8979879");

			//Header component
			if (HasComponent<HeaderComponent>())
			{
				SerializerHelper::Key("HeaderComponent");
				SerializationComponentHelper::SerializeHeaderComponent(&GetComponent<HeaderComponent>(),binary);
			}

			//Transform component
			if (HasComponent<TransformComponent>())
			{
				SerializerHelper::Key("TransformComponent");
				SerializationComponentHelper::SerializeTransformComponent(&GetComponent<TransformComponent>(),binary);
			}

			SerializerHelper::EndMap();

			return true;
		}

	}

	bool Entity::Deserialize(const YAML::Node& in, bool binary)
	{
		return false;
	}
#pragma endregion
}

