#include"czpch.h"
#include"HeaderComponent.h"

namespace Czuch
{
	HeaderComponent::HeaderComponent(Entity owner) :Component(owner)
	{
		m_Name = StringID::MakeStringID("Entity");
		m_Tag = StringID::MakeStringID(std::move("Default"));
	}

	HeaderComponent::HeaderComponent(Entity owner,const CzuchStr& name, const CzuchStr& tag, Layer layer)
		:Component(owner), m_Layer(layer)
	{
		m_Name = StringID::MakeStringID(std::move(name));
		m_Tag = StringID::MakeStringID(std::move(tag));
	}

}
