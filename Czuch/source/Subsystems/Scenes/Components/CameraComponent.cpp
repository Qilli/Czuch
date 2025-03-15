#include "czpch.h"
#include "CameraComponent.h"
#include"../Entity.h"

namespace Czuch
{
	CameraComponent::CameraComponent(Entity owner) :Component(owner),m_Camera(45.0f, (float)WindowInfo::Width/(float)WindowInfo::Height, 0.1f, 1000.0f)
	{
		m_Camera.SetTransform(&m_Owner.GetComponent<TransformComponent>());
	}

	void Camera::Recalculate()
	{
		m_ProjectionMatrix =glm::perspective(glm::radians(m_VerticalFov),m_AspectRatio,m_NearPlane,m_FarPlane);
		//m_ProjectionMatrix[1][1] *= -1;
	}

	Camera::Camera(float fov, float aspect, float nearPlane, float farPlane) :m_VerticalFov(fov), m_AspectRatio(aspect), m_NearPlane(nearPlane), m_FarPlane(farPlane)
	{
		Recalculate();
	}

	Camera::Camera() :Camera(45.0f, (float)WindowInfo::Width / (float)WindowInfo::Height, 0.1f, 1000.0f)
	{
		
	}

	Camera::~Camera()
	{
	}

	void Camera::SetTransform(TransformComponent* transform)
	{
		m_Transform = transform;
	}

	void Camera::SetVerticalFov(float fov)
	{
		m_VerticalFov = fov;
		Recalculate();
	}

	void Camera::SetNearPlane(float nearPlane)
	{
		m_NearPlane = nearPlane;
		Recalculate();
	}

	void Camera::SetFarPlane(float farPlane)
	{
		m_FarPlane = farPlane;
		Recalculate();
	}

	void Camera::SetAspectRatio(float aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		Recalculate();
	}

	void Camera::Set(float fov, float aspectRatio, float nearPlane, float farPlane)
	{
		m_VerticalFov = fov;
		m_AspectRatio = aspectRatio;
		m_NearPlane = nearPlane;
		m_FarPlane = farPlane;
		Recalculate();
	}

	Mat4x4 Camera::GetProjectionMatrix()
	{
		return m_ProjectionMatrix;
	}

	Mat4x4 Camera::GetInverseViewMatrix()
	{
		if (m_Transform!= nullptr)
		{
			return glm::inverse(m_Transform->GetLocalToWorld());
		}
		return Mat4x4(1.0f);
	}

	Mat4x4 Camera::GetViewMatrix()
	{
		return m_Transform->GetLocalToWorld();
	}

	Mat4x4 Camera::GetViewProjectionMatrix()
	{
		return m_ProjectionMatrix * GetInverseViewMatrix();
	}

}


