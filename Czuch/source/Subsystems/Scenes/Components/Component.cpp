#include "czpch.h"
#include "Component.h"
#include"../Entity.h"
#include"Renderer/DebugDraw.h"

namespace Czuch
{
	Component::Component(Entity owner) :m_State(true), m_Owner(owner),m_Enabled(true) {}
	void Component::OnDebugDraw(IDebugDrawBuilder* debugBuilder)
	{
	}
}

