#pragma once
#include"IScene.h"
#include"Renderer/Graphics.h"
#include "Serialization/ISerializer.h"

namespace Czuch
{

	struct TransformComponent;
	struct CameraComponent;
	class CZUCH_API Entity: public ISerializer
	{
	public:
		static Entity CreateInvalidEntity() { Entity e; e.Invalidate(); return e; }

		Entity():m_EntityHandle(entt::null),m_Scene(nullptr) {}
		Entity(const Entity&) = default;
		Entity(entt::entity handle, IScene* scene);
		~Entity() = default;

		void AddRenderable(AssetHandle model,MeshHandle meshHandle,AssetHandle overrideMaterial);
		void RemoveRenderable();

		void AddPointLight(const Color& color, float intensity, float range);
		void AddDirectionalLight(const Color& color, float intensity);
		void AddSpotLight(const Color& color, float intensity, float range, float innerAngle, float outerAngle);
		void RemoveLightComponent();


		bool IsRenderable();

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);

		template<typename T>
		T& GetComponent();

		template<typename T>
		bool HasComponent();

		template<typename T>
		void RemoveComponent();

		bool IsValid() const { return m_EntityHandle != entt::null; }
		void AddChild(Entity& entity);
		void SetAsParent(Entity& entity);

		bool operator == (const Entity& other) const;
		bool operator != (const Entity& other) const { return !(*this == other); }
		operator bool() const { return IsValid(); }

		void SetActive(bool isActive);
		inline void Invalidate() { m_EntityHandle = entt::null; }

		TransformComponent& Transform();
		intptr_t GetID() const { return (intptr_t)m_EntityHandle; }
		entt::entity GetRawEntityHandle() const { return m_EntityHandle; }
	public:
		virtual bool Serialize(YAML::Emitter& out, bool binary = false) override;
		virtual bool Deserialize(const YAML::Node& in, bool binary = false) override;
		bool IsDestroyed();
	public:
		void OnEachChild(std::function<void(Entity)> func,bool recursive,bool ignoreDestroyedComponent=false);
	public:
		IScene* GetScene() { return m_Scene; }

		template<typename T>
		Entity FindEntityWithComponent()
		{
			auto entt= m_Scene->FindEntityWithComponent<T>();
			return Entity(entt, m_Scene);
		}

	private:
		template<typename T, typename... Args>
		T& EmplaceComp(Args && ...args);

		template <typename T>
		void OnComponentRemoved(entt::registry& r, entt::entity e)
		{
			auto& comp = r.get<T>(e);
			comp.OnRemoved();
		}

		friend class Scene;
		entt::entity GetInternalHandle() const { return m_EntityHandle; }

	private:
		entt::entity m_EntityHandle;
		IScene* m_Scene;
	};

	template<typename T, typename ...Args>
	inline T& Entity::EmplaceComp(Args && ...args)
	{
		return m_Scene->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	inline T& Entity::AddComponent(Args && ...args)
	{
		CZUCH_BE_ASSERT(!HasComponent<T>(), "Entity already has component!");
		auto& comp = EmplaceComp<T>(*this, std::forward<Args>(args)...);
		if (std::is_same<T, CameraComponent>::value) 
		{
			m_Scene->CameraAdded((CameraComponent*) & comp);
		}

		comp.OnCreated();
		return comp;
	}

	template<typename T>
	inline void Entity::RemoveComponent()
	{
		if (!HasComponent<T>() || std::is_same<T, TransformComponent>::value)
		{
			return;
		}

		auto &comp = GetComponent<T>();
		if (std::is_same<T, CameraComponent>::value)
		{
			m_Scene->CameraRemoved((CameraComponent*)&comp);
		}

		comp.OnRemoved();
		m_Scene->GetRegistry().get<T>(m_EntityHandle).OnRemoved();
		m_Scene->GetRegistry().remove<T>(m_EntityHandle);
		m_Scene->Dirty();

	}

	template<typename T>
	inline T& Entity::GetComponent()
	{
		CZUCH_BE_ASSERT(HasComponent<T>(), "Entity does not have component!");
		return m_Scene->GetRegistry().get<T>(m_EntityHandle);
	}
	template<typename T>
	inline bool Entity::HasComponent()
	{
		auto* comp= m_Scene->GetRegistry().try_get<T>(m_EntityHandle);
		return comp != nullptr;
	}
}
