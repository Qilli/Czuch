#include "czpch.h"
#include "Component.h"
#include"../Entity.h"

namespace Czuch
{
	Component::Component(Entity owner) :m_State(true), m_Owner(owner),m_Enabled(true) {}
}

