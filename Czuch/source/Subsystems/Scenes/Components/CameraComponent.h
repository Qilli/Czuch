#pragma once
#include"TransformComponent.h"
#include"Core/Math.h"

namespace Czuch
{
	struct CZUCH_API Camera
	{
	public:
		Camera(float fov,float aspect,float nearPlane,float farPlane);
		Camera();
		~Camera();

		void SetTransform(TransformComponent *transform);
		void SetVerticalFov(float fov);
		void SetNearPlane(float near);
		void SetFarPlane(float far);
		inline float GetFov()const { return m_VerticalFov; }
		inline float GetNearPlane()const { return m_NearPlane; }
		inline float GetFarPlane()const { return m_FarPlane; }
		inline float GetAspectRatio()const { return m_AspectRatio; }
		void SetAspectRatio(float aspectRatio);
		void Set(float fov, float aspectRatio, float nearPlane, float farPlane);

		Mat4x4 GetProjectionMatrix();
		Mat4x4 GetViewMatrix();
		Mat4x4 GetInverseViewMatrix();
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
	};

	enum CameraType
	{
		EditorCamera=0,
		GameCamera=1
	};

	struct CZUCH_API CameraComponent : public Component
	{
	public:
		CameraComponent(Entity owner);
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(Entity owner, bool primary):Component(owner),m_Primary(primary),m_Type(CameraType::GameCamera){}
		~CameraComponent() = default;
		inline Camera& GetCamera() {return m_Camera;}
		inline void SetPrimaryFlag(bool primary) {
			m_Primary = primary;
		}
		inline void SetAsPrimary() { m_Owner.GetScene()->SetPrimaryCamera(this); }
		void SetAsEditorCamera() { m_Owner.GetScene()->SetEditorCamera(this); }
		inline bool IsPrimary() { return m_Primary; }
		inline void SetType(CameraType type) { m_Type = type; }
		inline CameraType GetType() { return m_Type; }
	private:
		Camera m_Camera;
		CameraType m_Type;
		bool m_Primary;
	};

}
