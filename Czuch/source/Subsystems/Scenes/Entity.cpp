#include "czpch.h"
#include "Entity.h"
#include"Components/TransformComponent.h"
#include"Components/MeshRendererComponent.h"
#include"Components/MeshComponent.h"
#include"Components/HeaderComponent.h"
#include"Components/CameraComponent.h"
#include"Serialization/SerializationComponentHelper.h"

namespace Czuch
{
	Entity::Entity(entt::entity handle, IScene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	void Entity::AddRenderable(AssetHandle model,MeshHandle meshHandle, AssetHandle overrideMaterial)
	{
		AddComponent<MeshComponent>(model,meshHandle);
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

	bool Entity::IsDestroyed()
	{
		return HasComponent<DestroyedComponent>();
	}

	void Entity::OnEachChild(std::function<void(Entity)> func,bool recursive,bool ignoreDestroyedComponent)
	{
		auto& transform = GetComponent<TransformComponent>();
		for (auto& child : transform.GetChildren())
		{
			if (child.IsValid() && (ignoreDestroyedComponent ==true||!child.IsDestroyed()))
			{
				func(child);
				if (recursive)
				{
					child.OnEachChild(func, recursive, ignoreDestroyedComponent);
				}
			}
		}
	}


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
			//GUID component
			if (HasComponent<GUIDComponent>())
			{
				SerializerHelper::Key("GUIDComponent");
				SerializationComponentHelper::SerializeGUIDComponent(&GetComponent<GUIDComponent>(), binary);
			}


			//Header component
			if (HasComponent<HeaderComponent>())
			{
				SerializerHelper::Key("HeaderComponent");
				SerializationComponentHelper::SerializeHeaderComponent(&GetComponent<HeaderComponent>(), binary);
			}

			//Active component
			if (HasComponent<ActiveComponent>())
			{
				SerializerHelper::Key("ActiveComponent");
				SerializerHelper::Value("1");
			}

			//Transform component
			if (HasComponent<TransformComponent>())
			{
				SerializerHelper::Key("TransformComponent");
				SerializationComponentHelper::SerializeTransformComponent(&GetComponent<TransformComponent>(), binary);
			}

			//Camera component
			if (HasComponent<CameraComponent>())
			{
				SerializerHelper::Key("CameraComponent");
				SerializationComponentHelper::SerializeCameraComponent(&GetComponent<CameraComponent>(), binary);
			}

			//Mesh component
			if (HasComponent<MeshComponent>())
			{
				SerializerHelper::Key("MeshComponent");
				SerializationComponentHelper::SerializeMeshComponent(&GetComponent<MeshComponent>(), binary);
			}

			//Mesh renderer component
			if (HasComponent<MeshRendererComponent>())
			{
				SerializerHelper::Key("MeshRendererComponent");
				SerializationComponentHelper::SerializeMeshRendererComponent(&GetComponent<MeshRendererComponent>(), binary);
			}

			SerializerHelper::EndMap();

			return true;
		}

	}

	bool Entity::Deserialize(const YAML::Node& in, bool binary)
	{
		if (binary)
		{
			return false;
		}

		auto entityNode = in;
		//uint64_t guid = entityNode.as<uint64_t>();
		if (entityNode["HeaderComponent"])
		{
			auto headerNode = entityNode["HeaderComponent"];
			auto &header = GetComponent<HeaderComponent>();
			bool headerComponentResult = SerializationComponentHelper::DeserializeHeaderComponent(&header, headerNode, binary);
			if (!headerComponentResult)
			{
				LOG_BE_ERROR("Failed to deserialize header component");
				return false;
			}
		}

		if (entityNode["GUIDComponent"])
		{
			auto guidNode = entityNode["GUIDComponent"];
			LOG_BE_INFO("Deserializing GUID component for entity: {0}",GetComponent<HeaderComponent>().GetHeader());
			auto &guid = GetComponent<GUIDComponent>();
			bool guidComponentResult = SerializationComponentHelper::DeserializeGUIDComponent(&guid, guidNode, binary);
			if (!guidComponentResult)
			{
				LOG_BE_ERROR("Failed to deserialize GUID component");
				return false;
			}
		}

		if (!entityNode["ActiveComponent"])
		{
			RemoveComponent<ActiveComponent>();
		}

		if (entityNode["TransformComponent"])
		{
			auto transformNode = entityNode["TransformComponent"];
			auto &transform = GetComponent<TransformComponent>();
			bool resultBaseComponent = SerializationComponentHelper::DeserializeBaseComponent(&transform, transformNode, binary);
			if (!resultBaseComponent)
			{
				LOG_BE_ERROR("Failed to deserialize base component of transform component");
				return false;
			}
			bool transformComponentResult = SerializationComponentHelper::DeserializeTransformComponent(&transform, transformNode, binary);
			if (!transformComponentResult)
			{
				LOG_BE_ERROR("Failed to deserialize transform component");
				return false;
			}
		}

		if (entityNode["CameraComponent"])
		{
			auto cameraNode = entityNode["CameraComponent"];
			auto &camera = AddComponent<CameraComponent>();
			bool resultBaseComponent = SerializationComponentHelper::DeserializeBaseComponent(&camera, cameraNode, binary);
			if (!resultBaseComponent)
			{
				LOG_BE_ERROR("Failed to deserialize base component of transform component");
				return false;
			}
			bool cameraComponentResult = SerializationComponentHelper::DeserializeCameraComponent(&camera, cameraNode, binary);
			if (!cameraComponentResult)
			{
				LOG_BE_ERROR("Failed to deserialize camera component");
				return false;
			}
		}

		if (entityNode["MeshComponent"])
		{
			auto meshNode = entityNode["MeshComponent"];
			auto& mesh = AddComponent<MeshComponent>();
			bool resultBaseComponent = SerializationComponentHelper::DeserializeBaseComponent(&mesh, meshNode, binary);
			if (!resultBaseComponent)
			{
				LOG_BE_ERROR("Failed to deserialize base component of mesh component");
				return false;
			}
			bool meshComponentResult = SerializationComponentHelper::DeserializeMeshComponent(&mesh, meshNode, binary);
			if (!meshComponentResult)
			{
				LOG_BE_ERROR("Failed to deserialize mesh component");
				return false;
			}
		}

		if (entityNode["MeshRendererComponent"])
		{
			auto meshRendererNode = entityNode["MeshRendererComponent"];
			auto& meshRenderer = AddComponent<MeshRendererComponent>();
			bool resultBaseComponent = SerializationComponentHelper::DeserializeBaseComponent(&meshRenderer, meshRendererNode, binary);
			if (!resultBaseComponent)
			{
				LOG_BE_ERROR("Failed to deserialize base component of mesh renderer component");
				return false;
			}
			bool meshRendererComponentResult = SerializationComponentHelper::DeserializeMeshRendererComponent(&meshRenderer, meshRendererNode, binary);
			if (!meshRendererComponentResult)
			{
				LOG_BE_ERROR("Failed to deserialize mesh renderer component");
				return false;
			}
		}


		return true;
	}
#pragma endregion
}

