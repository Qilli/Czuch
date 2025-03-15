#pragma once
#include"Entity.h"
#include"Core/TimeDelta.h"
#include"./Events/IEventsListener.h"

namespace Czuch
{
	enum CZUCH_API ScriptRunningMode
	{
		EditorOnly,
		PlayMode,
		EditorAndPlayMode
	};

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
		void OnEvent(Event& event) override  {}

		void SetEntity(Entity entity) { m_Entity = entity; }
		Entity GetEntity() { return m_Entity; }

		inline void SetEnabled(bool enabled) { m_IsEnabled = enabled; }
		inline bool IsEnabled() { return m_IsEnabled; }
		inline void SetRunningMode(ScriptRunningMode mode) { m_RunningMode = mode; }
		inline ScriptRunningMode GetRunningMode() { return m_RunningMode; }
	private:
		friend class Scene;
		ScriptRunningMode m_RunningMode = ScriptRunningMode::PlayMode;
		Entity m_Entity;
		bool m_IsEnabled = true;
	};

}


