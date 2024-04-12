#pragma once
#include"TransformComponent.h"
#include"Core/Math.h"

namespace Czuch
{
	struct Camera
	{
	public:
		Camera(float fov,float aspect,float nearPlane,float farPlane);
		Camera();
		~Camera();

		void SetTransform(TransformComponent *transform);
		void SetVerticalFov(float fov);
		void SetNearPlane(float near);
		void SetFarPlane(float far);
		void SetAspectRatio(float aspectRatio);
		void Set(float fov, float aspectRatio, float nearPlane, float farPlane);

		Mat4x4 GetProjectionMatrix();
		Mat4x4 GetViewMatrix();
		Mat4x4 GetViewProjectionMatrix();
	private:
		void Recalculate();
	private:
		TransformComponent *m_Transform;
		Mat4x4  m_ProjectionMatrix;
		float m_VerticalFov;
		float m_AspectRatio;
		float m_NearPlane;
		float m_FarPlane;
		float m_HorizontalFov;
	};

	struct CameraComponent : public Component
	{
	public:
		CameraComponent(Entity owner);
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(Entity owner, bool primary):Component(owner),m_Primary(primary){}
		~CameraComponent() = default;
		inline Camera& GetCamera() {return m_Camera;}
		inline void SetPrimary(bool primary) { m_Primary = primary; }
		inline bool IsPrimary() { return m_Primary; }
	private:
		Camera m_Camera;
		bool m_Primary;
	};

}
