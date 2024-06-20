#pragma once
#include "../NativeBehaviour.h"

namespace Czuch
{
    class CZUCH_API NativeFree3DCameraController :
        public NativeBehaviour
    {
    public:
		NativeFree3DCameraController(Entity entity);
		NativeFree3DCameraController() = default;
		~NativeFree3DCameraController();

		void SetSpeed(float speed) { m_Speed = speed; }
		void SetSensitivity(float sensitivity) { m_Sensitivity = sensitivity; }

		void OnUpdate(TimeDelta delta) override;
		void OnCreate() override;
		void OnEnable() override;
		void OnDisable() override;
		void OnEvent(Event& event) override;
	private:
		float m_Speed = 1.0f;
		float m_Sensitivity = 0.003f;
		TransformComponent* m_CameraTransformComponent;
    };
}
;

