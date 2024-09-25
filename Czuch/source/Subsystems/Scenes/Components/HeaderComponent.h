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
		void SetVisibleInEditorHierarchy(bool visible) { m_VisibleInEditorHierarchy = visible; }
		bool IsVisibleInEditorHierarchy() const { return m_VisibleInEditorHierarchy; }
	private:
		bool m_VisibleInEditorHierarchy = true;
		StringID m_Name;
		StringID m_Tag;
		Layer m_Layer;
	};
}