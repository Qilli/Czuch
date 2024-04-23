#pragma once
#include"Core/Math.h"
#include"Component.h"


namespace Czuch
{
	class Entity;
	struct CZUCH_API TransformComponent:Component
	{
	public:
		TransformComponent(Entity owner);
		TransformComponent(const TransformComponent&) = default;

		Mat4x4 GetLocalToWorld();
		const Mat4x4 GetParentTransform();
		void SetLocalPosition(const Vec3& position);
		void SetLocalEulerAngles(const Vec3& eulerAngles);

		void SetLocalScale(const Vec3& scale);
		void SetParentTransform(const Mat4x4& transform);

		const Vec3& GetLocalPosition() const { return m_LocalPosition; }
		const Vec3& GetLocalEulerAngles() const { return m_LocalEulerAngles; }
		const Vec3& GetLocalScale() const { return m_LocalScale; }
		const Mat4x4& GetLocalTransform() const { return m_LocalTransform; }
		const ComponentState& GetState() const { return m_State; }
		void SetDirty() { m_State.SetDirty(); }

		const Vec3 GetForward() const { return glm::normalize(m_LocalTransform[2]); }
		const Vec3 GetRight() const { return glm::normalize(m_LocalTransform[0]); }
		const Vec3 GetUp() const { return glm::normalize(m_LocalTransform[1]); }

		void Translate(const Vec3& translation, TransformSpace space = TransformSpace::Local);
		void Rotate(float angle,Vec3 axis=Vec3(0,1,0), TransformSpace space = TransformSpace::Local);
		void Scale(const Vec3& scale);

		std::vector<Entity>& GetChildren() { return m_Children; }

	public:
		void SetParent(Entity parent);
		void AddChild(Entity child);
		void UpdateLocalToWorld();
	private:
		void SetParentInternal(Entity entity);
		void SetChildInternal(Entity entity);
		void RemoveChildInternal(Entity entity);

	private:
		void UpdateLocalTransform();
	private:
		std::vector<Entity> m_Children;
		Entity m_Parent;
		Mat4x4 m_LocalToWorld;
		Mat4x4 m_LocalTransform;
		Mat4x4 m_ParentTransform;
		glm::quat m_LocalRotation;
		Vec3 m_LocalPosition;
		Vec3 m_LocalEulerAngles;
		Vec3 m_LocalScale;
	};
}