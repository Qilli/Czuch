#pragma once
#include"ISerializer.h"

namespace Czuch
{
	struct HeaderComponent;
	struct Component;
	struct TransformComponent;
	struct AdtiveComponent;
	struct CameraComponent;
	struct GUIDComponent;
	struct Camera;


	class SerializationComponentHelper
	{
	public:
		static bool SerializeBaseComponent(Component* component, bool binary);
		static bool SerializeHeaderComponent(HeaderComponent* component, bool binary);
		static bool SerializeTransformComponent(TransformComponent* component, bool binary);
		static bool SerializeCameraComponent(CameraComponent* component, bool binary);
		static bool SerializeGUIDComponent(GUIDComponent* component, bool binary);
		static bool SerializeCamera(Camera* camera, bool binary);


		static bool DeserializeBaseComponent(Component* component, const YAML::Node& in, bool binary);
		static bool DeserializeHeaderComponent(HeaderComponent* component, const YAML::Node& in, bool binary);
		static bool DeserializeTransformComponent(TransformComponent* component, const YAML::Node& in, bool binary);
		static bool DeserializeCameraComponent(CameraComponent* component, const YAML::Node& in, bool binary);
		static bool DeserializeGUIDComponent(GUIDComponent* component, const YAML::Node& in, bool binary);
		static bool DeserializeCamera(Camera* camera, const YAML::Node& in, bool binary);
	};
}


