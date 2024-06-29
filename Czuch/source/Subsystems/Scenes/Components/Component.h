#pragma once
#include"EngineRoot.h"
#include"ComponentState.h"
#include"../Entity.h"
#include"Core/UpdatableValue.h"

namespace Czuch
{
	struct Layer
	{
		U32 m_Layer = 0;
	};

	enum class TransformSpace
	{
		Local,
		World
	};

	struct Component
	{
	public:
		Component(Entity owner);
		Component(const Component&) = default;
		ComponentState& GetState() { return m_State; }
		bool IsEnabled() { return m_Enabled.GetValue(); }
		Entity GetEntity() { return m_Owner; }
		virtual void SetEnabled(bool enabled) { m_Enabled.UpdateValue(enabled); }
		virtual void OnCreated() {}
		virtual void OnRemoved() {}
		virtual void OnFinishFrame() {}
	private:

	protected:
		friend class Entity;
		ComponentState m_State;
		Entity m_Owner;
		UpdatableValue<bool> m_Enabled;
	};

	struct ActiveComponent : public Component
	{
	public:
		ActiveComponent(Entity owner) : Component::Component(owner) {}
		ActiveComponent(const ActiveComponent&) = default;
		virtual ~ActiveComponent() = default;
	};
}
