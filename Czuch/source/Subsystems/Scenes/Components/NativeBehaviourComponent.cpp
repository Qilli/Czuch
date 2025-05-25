#include "czpch.h"
#include "NativeBehaviourComponent.h"
#include "./Subsystems/Scenes/NativeBehaviour.h"
#include "Renderer/DebugDraw.h"

namespace Czuch
{
	NativeBehaviourComponent::NativeBehaviourComponent(Entity owner)
		:Component(owner), m_BehavioursCount(0)
	{
		
	}

	NativeBehaviourComponent::~NativeBehaviourComponent()
	{
		OnDestroy();
	}

	void NativeBehaviourComponent::SetEnabled(bool enabled)
	{
		bool currentStatus = IsEnabled();
		Component::SetEnabled(enabled);

		if (currentStatus == enabled)
		{
			return;
		}

		if (enabled)
		{
			OnEnable();
		}
		else
		{
			OnDisable();
		}
	}

	void NativeBehaviourComponent::OnDebugDraw(IDebugDrawBuilder* debugBuilder)
	{
		if (!IsEnabled())
		{
			return;
		}

		for (size_t i = 0; i < m_BehavioursCount; i++)
		{
			if (m_NativeBehaviours[i].behaviour->IsEnabled())
			{
				m_NativeBehaviours[i].behaviour->OnDebugDraw(debugBuilder);
			}
		}
	}

	void NativeBehaviourComponent::OnCreate()
	{
		for (size_t i = 0; i < m_BehavioursCount; i++)
		{
			m_NativeBehaviours[i].behaviour->OnCreate();
		}
	}

	void NativeBehaviourComponent::OnDestroy()
	{
		for (size_t i = 0; i < m_BehavioursCount; i++)
		{
			m_NativeBehaviours[i].behaviour->OnDisable();
			m_NativeBehaviours[i].behaviour->OnDestroy();
			delete m_NativeBehaviours[i].behaviour;
		}
		m_BehavioursCount = 0;
	}

	void NativeBehaviourComponent::OnUpdate(TimeDelta delta)
	{
		if (!IsEnabled())
		{
			return;
		}
		EngineMode mode = EngineRoot::Get().GetGameMode();
		EngineStateMode state = EngineRoot::Get().GetEngineStateMode();

		if (EngineRoot::Get().GetUpdateMode() == UpdateMode::Locked)
		{
			return;
		}

		for (size_t i = 0; i < m_BehavioursCount; i++)
		{
			if (mode == EngineMode::Editor)
			{

				if (state == EngineStateMode::Editor && m_NativeBehaviours[i].behaviour->GetRunningMode() == ScriptRunningMode::PlayMode)
				{
					continue;
				}

				if (state == EngineStateMode::Playmode && m_NativeBehaviours[i].behaviour->GetRunningMode() == ScriptRunningMode::EditorOnly)
				{
					continue;
				}
			}
			else
			{
				if (m_NativeBehaviours[i].behaviour->GetRunningMode() == ScriptRunningMode::EditorOnly)
				{
					continue;
				}
			}

			if (m_NativeBehaviours[i].behaviour->IsEnabled())
			{
				m_NativeBehaviours[i].behaviour->OnUpdate(delta);
			}
		}
	}

	void NativeBehaviourComponent::OnEnable()
	{
		for (size_t i = 0; i < m_BehavioursCount; i++)
		{
			m_NativeBehaviours[i].behaviour->OnEnable();
		}
	}

	void NativeBehaviourComponent::OnDisable()
	{
		for (size_t i = 0; i < m_BehavioursCount; i++)
		{
			m_NativeBehaviours[i].behaviour->OnDisable();
		}
	}

	void NativeBehaviourComponent::InitNewBehaviour(NativeBehaviour* behaviour)
	{
		behaviour->SetEntity(m_Owner);
		behaviour->OnCreate();
		behaviour->OnEnable();
	}
}
