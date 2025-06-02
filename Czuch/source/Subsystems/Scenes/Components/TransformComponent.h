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
		void SetLocalTransform(const Mat4x4& transform);

		void SetLocalScale(const Vec3& scale);
		void SetParentTransform(const Mat4x4& transform);
		void LookAt(const Vec3& target);

		Vec3& GetLocalPosition() { return m_LocalPosition; }
		Vec3& GetLocalEulerAngles(){ return m_LocalEulerAngles; }
		Vec3& GetLocalScale() { return m_LocalScale; }
		const Mat4x4& GetLocalTransform() const { return m_LocalTransform; }
		const ComponentState& GetState() const { return m_State; }
		void SetDirty() { m_State.SetDirty(); }
		void ForceUpdateLocalTransform() { UpdateLocalRotation(); UpdateLocalTransform(); SetDirty(); }

		const Vec3 GetForward() const { return glm::normalize(m_LocalTransform[2]); }
		const Vec3 GetRight() const { return glm::normalize(m_LocalTransform[0]); }
		const Vec3 GetUp() const { return glm::normalize(m_LocalTransform[1]); }

		const Vec3 GetWorldForward() const { return glm::normalize(m_LocalToWorld[2]); }
		const Vec3 GetWorldRight() const { return glm::normalize(m_LocalToWorld[0]); }
		const Vec3 GetWorldUp() const { return glm::normalize(m_LocalToWorld[1]); }

		const Vec3 GetInvWorldForward() const { return glm::normalize(glm::inverse(m_LocalToWorld)[2]); }
		const Vec3 GetInvWorldRight() const { return glm::normalize(glm::inverse(m_LocalToWorld)[0]); }
		const Vec3 GetInvWorldUp() const { return glm::normalize(glm::inverse(m_LocalToWorld)[1]); }


		const Mat3x3 GetWorldInv3x3()
		{
			UpdateLocalToWorld();
			auto inv = glm::inverse(m_LocalToWorld);
			return Mat3x3(inv);
		}

		const Mat4x4 GetInverseTransposeLocalToWorld()
		{
			return glm::inverse(glm::transpose(m_LocalToWorld));
		}

		const Mat4x4 GetWorldInv4x4()
		{
			UpdateLocalToWorld();
			auto inv = glm::inverse(m_LocalToWorld);
			return Mat4x4(inv);
		}

		const Vec3 TransformPointToLocalSpace(Vec3 vec)
		{
			return GetWorldInv3x3() * vec;
		}

		const Mat4x4 TransformToLocalSpace(Mat4x4& mat)
		{
			return GetWorldInv4x4() * mat;
		}

		void Translate(const Vec3& translation, TransformSpace space = TransformSpace::Local);
		void Rotate(float angle,Vec3 axis=Vec3(0,1,0), TransformSpace space = TransformSpace::Local);
		void Scale(const Vec3& scale);

		Vec3 GetWorldPosition();

		std::vector<Entity>& GetChildren() { return m_Children; }
		bool HasAnyChild();

	public:
		void SetParent(Entity parent);
		GUID GetParentGUID() { return m_Parent.IsValid()?m_Parent.GetComponent<GUIDComponent>().GetGUID():INVALID_GUID; }
		void SetParentFromGUID(GUID guid);
		void AddChild(Entity child);
		void UpdateLocalToWorld();
	private:
		void SetParentInternal(Entity entity);
		void SetChildInternal(Entity entity);
		void RemoveChildInternal(Entity entity);

	private:
		void UpdateLocalTransform();
		void UpdateLocalRotation();
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