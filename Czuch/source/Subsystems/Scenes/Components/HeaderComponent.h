#pragma once
#include"Component.h"
#include"Core/UpdatableValue.h"

namespace Czuch
{
	struct HeaderComponent : public Component
	{
	public:
		HeaderComponent(Entity owner);
		HeaderComponent(const HeaderComponent&) = default;
		HeaderComponent(Entity owner,const CzuchStr& name, const CzuchStr& tag, Layer layer);
		const CzuchStr& GetHeader() const { return m_Name.GetStrName(); }
	private:
		StringID m_Name;
		StringID m_Tag;
		Layer m_Layer;
	};
}