#include"czpch.h"
#include "LightComponent.h"

namespace Czuch
{
	LightComponent::LightComponent(Entity owner) : Component(owner)
	{
		m_LightRange = 10.0f;
		m_Color = Colors::White;
		m_InnerAngle = 30.0f;
		m_OuterAngle = 32.0f;
		m_Type = LightType::Directional;
	}

	LightComponent::LightComponent(Entity owner, LightType type) : Component(owner), m_Type(type)
	{
		m_LightRange = 10.0f;
		m_Color = Colors::White;
		m_InnerAngle = 30.0f;
		m_OuterAngle = 32.0f;
		m_Type = LightType::Directional;
	}
}