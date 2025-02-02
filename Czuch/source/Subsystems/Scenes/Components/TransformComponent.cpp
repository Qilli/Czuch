#include "czpch.h"
#include "TransformComponent.h"
#include"../Entity.h"

namespace Czuch
{
	TransformComponent::TransformComponent(Entity owner) :Component(owner)
	{
		m_LocalPosition = Vec3(0.0f);
		m_LocalEulerAngles = Vec3(0.0f);
		m_LocalScale = Vec3(1.0f);
		m_LocalRotation = glm::quat(1, 0, 0, 0);
		m_LocalTransform = Mat4x4(1.0f);
		m_LocalToWorld = Mat4x4(1.0f);
		m_ParentTransform = Mat4x4(1.0f);
		m_Parent = Entity();
		m_State.SetDirty();
	}

	Mat4x4 TransformComponent::GetLocalToWorld()
	{
		if (m_State.IsDirty())
		{
			m_LocalToWorld = GetParentTransform() * m_LocalTransform;
			m_State.SetAsClean();

			if (HasAnyChild())
			{
				for (size_t i = 0; i < m_Children.size(); i++)
				{
					if (m_Children[i].IsValid() && !m_Children[i].IsDestroyed())
					{
						auto& childTransform = m_Children[i].GetComponent<TransformComponent>();
						childTransform.UpdateLocalToWorld();
					}
				}
			}	
		}
		return m_LocalToWorld;
	}

	const Mat4x4 TransformComponent::GetParentTransform()
	{
		if (m_Parent.IsValid())
		{
			auto& transformParent=m_Parent.GetComponent<TransformComponent>();
			return transformParent.GetLocalToWorld();
		}
		return Mat4x4(1.0f);
	}

	void TransformComponent::SetLocalPosition(const Vec3& position)
	{
		m_LocalPosition = position;
		m_LocalTransform[3] = Vec4(m_LocalPosition, 1.0f);
		m_State.SetDirty();
	}

	void TransformComponent::SetLocalEulerAngles(const Vec3& eulerAngles)
	{
		m_LocalEulerAngles = eulerAngles;
		ForceUpdateLocalTransform();
	}

	void TransformComponent::SetLocalTransform(const Mat4x4& transform)
	{
		m_LocalTransform = transform;
		m_LocalPosition = Vec3(m_LocalTransform[3]);
		m_LocalRotation = glm::quat_cast(m_LocalTransform);
		m_LocalScale = Vec3(glm::length(m_LocalTransform[0]), glm::length(m_LocalTransform[1]), glm::length(m_LocalTransform[2]));
		m_LocalEulerAngles = glm::degrees(glm::eulerAngles(m_LocalRotation));
		m_State.SetDirty();

	}

	void TransformComponent::SetLocalScale(const Vec3& scale)
	{
		m_LocalScale = scale;
		UpdateLocalTransform();
		m_State.SetDirty();
	}

	void TransformComponent::SetParentTransform(const Mat4x4& transform)
	{
		m_ParentTransform = transform;
		m_State.SetDirty();
		UpdateLocalToWorld();
	}

	Vec3 TransformComponent::GetWorldPosition()
	{
		return Vec3(GetLocalToWorld()[3]);
	}

	void TransformComponent::Translate(const Vec3& translation, TransformSpace space)
	{
		if (space == TransformSpace::Local)
		{
			m_LocalPosition+= GetForward() * translation.z;
			m_LocalPosition+= GetRight() * translation.x;
			m_LocalPosition+= GetUp() * translation.y;
		}
		else
		{
			//compute inv with parent forward/right/up
			auto & transformParent = m_Parent.GetComponent<TransformComponent>();
			m_LocalPosition += transformParent.GetInvWorldForward() * translation.z;
			m_LocalPosition += transformParent.GetInvWorldRight() * translation.x;
			m_LocalPosition += transformParent.GetInvWorldUp() * translation.y;
		}
		m_LocalTransform[3] = Vec4(m_LocalPosition, 1.0f);
		m_State.SetDirty();
	}

	void TransformComponent::Rotate(float angle, Vec3 axis, TransformSpace space)
	{
		if (space == TransformSpace::Local)
		{
			m_LocalRotation = glm::rotate(m_LocalRotation, angle, axis);
		}
		else
		{
			m_LocalRotation = m_LocalRotation*glm::rotate(glm::quat(1,0,0,0), angle, axis);
		}
		m_LocalEulerAngles = glm::degrees(glm::eulerAngles(m_LocalRotation));
		UpdateLocalRotation();
		UpdateLocalTransform();
		m_State.SetDirty();
	}

	void TransformComponent::Scale(const Vec3& scale)
	{
		m_LocalScale *= scale;
		UpdateLocalTransform();
		m_State.SetDirty();
	}

	void Czuch::TransformComponent::SetParent(Entity parent)
	{
		if (parent == m_Parent)
		{
			return;
		}

		if (!m_Parent.IsValid())
		{
			SetParentInternal(parent);
			auto& parentTransform = m_Parent.GetComponent<TransformComponent>();
			parentTransform.SetChildInternal(m_Owner);
			m_State.SetDirty();
			return;
		}

		auto& oldParent = m_Parent.GetComponent<TransformComponent>();
		oldParent.RemoveChildInternal(m_Owner);
		SetParentInternal(parent);
		if (!m_Parent.IsValid())
		{
			m_State.SetDirty();
			return;
		}
		auto& parentTransform = m_Parent.GetComponent<TransformComponent>();
		parentTransform.SetChildInternal(m_Owner);
		m_State.SetDirty();
	}

	void TransformComponent::SetParentFromGUID(GUID guid)
	{
		if (!guid.IsValid())
		{
			//root entity has no parent
			return;
		}
		entt::entity parent = m_Owner.GetScene()->GetEntityWithGUID(guid);
		SetParent(Entity(parent,m_Owner.GetScene()));
	}

	bool TransformComponent::HasAnyChild()
	{
		if (m_Children.size() == 0)
		{
			return false;
		}
		
		for (size_t i = 0; i < m_Children.size(); i++)
		{
			if (m_Children[i].IsValid() && !m_Children[i].IsDestroyed())
			{
				return true;
			}
		}
		return false;
	}

	void TransformComponent::AddChild(Entity child)
	{
		bool isChild = false;
		for (size_t i = 0; i < m_Children.size(); i++)
		{
			if (m_Children[i] == child) {
				isChild = true;
			}
		}
	
		if (!isChild)
		{
			m_Children.push_back(child);
			auto& childTransform = child.GetComponent<TransformComponent>();
			
			if (childTransform.m_Parent.IsValid())
			{
				childTransform.m_Parent.GetComponent<TransformComponent>().RemoveChildInternal(child);
			}

			childTransform.SetParentInternal(m_Owner);
		}
		m_State.SetDirty();
	}

	void TransformComponent::SetParentInternal(Entity entity)
	{
		m_Parent = entity;
		m_State.SetDirty();
	}

	void TransformComponent::SetChildInternal(Entity entity)
	{
		m_Children.push_back(entity);
	}

	void TransformComponent::RemoveChildInternal(Entity entity)
	{
		if (!entity.IsValid())
		{
			return;
		}

		for (size_t i = 0; i < m_Children.size(); i++)
		{
			if (m_Children[i] == entity)
			{
				m_Children.erase(m_Children.begin() + i);
				break;
			}
		}
	}

	void TransformComponent::UpdateLocalTransform()
	{
		m_LocalTransform = glm::translate(Mat4x4(1.0f), m_LocalPosition) * glm::toMat4(m_LocalRotation) * glm::scale(Mat4x4(1.0f), m_LocalScale);
	}

	void TransformComponent::UpdateLocalRotation()
	{
		if (m_LocalEulerAngles.x > 360.0f)
			m_LocalEulerAngles.x = fmod(m_LocalEulerAngles.x, 360.0f);
		if (m_LocalEulerAngles.y > 360.0f)
			m_LocalEulerAngles.y = fmod(m_LocalEulerAngles.y, 360.0f);
		if (m_LocalEulerAngles.z > 360.0f)
			m_LocalEulerAngles.z = fmod(m_LocalEulerAngles.z, 360.0f);

		m_LocalRotation = glm::quat(glm::vec3(glm::radians(m_LocalEulerAngles.x), glm::radians(m_LocalEulerAngles.y), glm::radians(m_LocalEulerAngles.z)));
	}

	void TransformComponent::UpdateLocalToWorld()
	{
		if (m_Parent.IsValid())
		{
			m_LocalToWorld = m_Parent.GetComponent<TransformComponent>().GetLocalToWorld() * m_LocalTransform;
		}
		else
		{
			m_LocalToWorld = m_LocalTransform;
		}
	}
}


