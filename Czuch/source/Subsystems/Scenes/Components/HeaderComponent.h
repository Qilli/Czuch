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
		const CzuchStr& GetTag() const { return m_Tag.GetStrName(); }
		Layer GetLayer() const { return m_Layer; }
		void SetHeader(const CzuchStr& name) { m_Name = StringID::MakeStringID(name); }
		void SetTag(const CzuchStr& tag) { m_Tag = StringID::MakeStringID(tag); }
	private:
		StringID m_Name;
		StringID m_Tag;
		Layer m_Layer;
	};
}