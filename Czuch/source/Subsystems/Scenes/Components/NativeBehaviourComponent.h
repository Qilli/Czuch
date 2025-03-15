#pragma once
#include"Component.h"
#include"./Core/TimeDelta.h"

namespace Czuch
{
	static const U8 s_max_native_behaviours_per_entity = 16;

	class NativeBehaviour;

	struct NativeBehaviourContainer
	{
		NativeBehaviour* behaviour;
		bool IsInited() const { return behaviour != nullptr; };
	};

	class CZUCH_API NativeBehaviourComponent : public Component
	{
	public:
		NativeBehaviourComponent(Entity owner);
		~NativeBehaviourComponent();

		void SetEnabled(bool enabled) override;
	public:
		void OnCreate();
		void OnDestroy();
		void OnUpdate(TimeDelta delta);
		void OnEnable();
		void OnDisable();
	public:
		template <typename T>
		T& AddNativeBehaviour()
		{
			CZUCH_BE_ASSERT(m_BehavioursCount < s_max_native_behaviours_per_entity - 1, "This native behaviours has max amount of native scripts");
			m_NativeBehaviours[m_BehavioursCount].behaviour = new T();
			InitNewBehaviour(m_NativeBehaviours[m_BehavioursCount].behaviour);
			return static_cast<T&>(*m_NativeBehaviours[m_BehavioursCount++].behaviour);
		}

		template <typename T>
		void RemoveNativeBehaviour()
		{
			int index = -1;
			for (U32 i = 0; i < m_BehavioursCount; i++)
			{
				if (dynamic_cast<T*>(m_NativeBehaviours[i].behaviour))
				{
					delete m_NativeBehaviours[i].behaviour;
					m_NativeBehaviours[i].behaviour = nullptr;
					index = i;
					break;
				}
			}

			if (index != -1)
			{
				for (U32 i = index; i < m_BehavioursCount - 1; i++)
				{
					m_NativeBehaviours[i] = m_NativeBehaviours[i + 1];
				}
				m_BehavioursCount--;
			}
		}

	private:
		void InitNewBehaviour(NativeBehaviour* beaviour);

	private:
		NativeBehaviourContainer m_NativeBehaviours[s_max_native_behaviours_per_entity];
		U32 m_BehavioursCount;
	};
}


