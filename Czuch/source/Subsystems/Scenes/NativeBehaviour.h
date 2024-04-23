#pragma once
#include"Entity.h"
#include"Core/TimeDelta.h"
#include"./Events/IEventsListener.h"

namespace Czuch
{
	class Scene;
	class CZUCH_API NativeBehaviour : public IEventsListener
	{
	public:
		NativeBehaviour(): m_Entity(),m_IsEnabled(true) {}

		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

		template<typename T>
		Entity FindEntityWitComponent()
		{
			return m_Entity->FindEntityWithComponent<T>();
		}

		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(TimeDelta delta) {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}
		void OnEvent(const Event& event) override  {}

		void SetEntity(Entity entity) { m_Entity = entity; }
		Entity GetEntity() { return m_Entity; }

		inline void SetEnabled(bool enabled) { m_IsEnabled = enabled; }
		inline bool IsEnabled() { return m_IsEnabled; }
	private:
		friend class Scene;
		Entity m_Entity;
		bool m_IsEnabled = true;
	};

}


