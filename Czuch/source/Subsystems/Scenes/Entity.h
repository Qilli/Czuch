#pragma once
#include"IScene.h"
#include"Renderer/Graphics.h"

namespace Czuch
{

	struct TransformComponent;
	class Entity
	{
	public:
		static Entity CreateInvalidEntity() { Entity e; e.Invalidate(); return e; }

		Entity() = default;
		Entity(const Entity&) = default;
		Entity(entt::entity handle, IScene* scene);
		~Entity() = default;

		void AddRenderable(MeshHandle meshHandle,MaterialInstanceHandle overrideMaterial);
		void RemoveRenderable();

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
	private:

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
	inline T& Entity::AddComponent(Args && ...args)
	{
		CZUCH_BE_ASSERT(!HasComponent<T>(), "Entity already has component!");
		auto& comp = m_Scene->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
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
		GetComponent<T>().OnRemoved();
		m_Scene->GetRegistry().get<T>(m_EntityHandle).OnRemoved();
		m_Scene->GetRegistry().remove<T>(m_EntityHandle);
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
