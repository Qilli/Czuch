#pragma once
#include "Component.h"
#include "Core/Math.h"


namespace Czuch
{
	enum class CZUCH_API LightType: int
	{
		Directional = 0,
		Point = 1,
		Spot = 2,
		Count = 3
	};

	constexpr int ToInt(LightType type) { return static_cast<int>(type); }
	constexpr const char* DirectionalLightTypeStr = "Directional";
	constexpr const char* PointLightTypeStr = "Point";
	constexpr const char* SpotLightTypeStr = "Spot";
	constexpr const char* UnknownLightTypeStr = "Unknown";

	constexpr const char* GetLightTypeString(LightType type)
	{
		switch (type)
		{
		case LightType::Directional:
			return DirectionalLightTypeStr;
		case LightType::Point:
			return PointLightTypeStr;
		case LightType::Spot:
			return SpotLightTypeStr;
		default:
			return UnknownLightTypeStr;
		}
	}


	class CZUCH_API LightComponent :
		public Component
	{
	public:
		LightComponent(Entity owner);
		LightComponent(Entity owner, LightType type);
		~LightComponent() = default;

		inline LightType GetLightType() const { return m_Type; }
		void SetLightType(LightType type) { m_Type = type; }

		inline float GetLightRange() const { return m_LightRange; }
		void SetLightRange(float range) { m_LightRange = range; }

		inline float GetLightIntensity() const { return m_LightIntensity; }
		void SetLightIntensity(float intensity) { m_LightIntensity = intensity; }

		inline Color GetColor() const { return GammaToLinear(m_Color); }
		void SetColor(Color color) { m_Color = color; }

		inline float GetInnerAngle() const { return m_InnerAngle; }
		inline float GetInnerAngleCos() const { return cosf(m_InnerAngle*DEG2RAD); }
		//in degrees
		void SetInnerAngle(float angle) { m_InnerAngle = angle; }

		inline float GetOuterAngle() const { return m_OuterAngle; }
		inline float GetOuterAngleCos() const { return cosf(m_OuterAngle*DEG2RAD); }
		//in degrees
		void SetOuterAngle(float angle) { m_OuterAngle = angle; }

		void OnRemoved() override {
		
		}
	private:
		Color m_Color;
		LightType m_Type;
		float m_LightIntensity;
		float m_LightRange;
		float m_SpecularPower;
		float m_InnerAngle;//radians
		float m_OuterAngle;//radians
	};
}