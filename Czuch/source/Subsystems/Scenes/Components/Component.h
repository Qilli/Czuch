#pragma once
#include"EngineRoot.h"
#include"ComponentState.h"
#include"../Entity.h"
#include"Core/UpdatableValue.h"
#include"Core/GUID.h"

namespace Czuch
{
	class IDebugDrawBuilder;
	struct Layer
	{
		U32 m_Layer = 0;
	};

	enum class TransformSpace
	{
		Local,
		World
	};

	struct CZUCH_API Component
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
		virtual void OnDebugDraw(IDebugDrawBuilder* debugBuilder);
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

	struct CZUCH_API GUIDComponent : public Component
	{
	public:
		GUIDComponent(Entity owner, GUID guid) : Component::Component(owner), m_GUID(guid) {}
		GUIDComponent(const GUIDComponent&) = default;
		virtual ~GUIDComponent() = default;
		GUID GetGUID() { return m_GUID; }
		void SetGUID(GUID guid) { m_GUID = guid; }
	private:
		GUID m_GUID;
	};

	struct CZUCH_API DestroyedComponent : public Component
	{
	public:
		DestroyedComponent(Entity owner) : Component::Component(owner) {}
		DestroyedComponent(const DestroyedComponent&) = default;
		virtual ~DestroyedComponent() = default;
	};

}
