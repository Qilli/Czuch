#include "czpch.h"
#include "SerializationComponentHelper.h"
#include"../Subsystems/Scenes/Components/TransformComponent.h"
#include"../Subsystems/Scenes/Components/HeaderComponent.h"
#include"../Subsystems/Scenes/Components/CameraComponent.h"
#include"../Subsystems/Scenes/Components/MeshComponent.h"
#include"../Subsystems/Scenes/Components/MeshRendererComponent.h"
#include"../Subsystems/Scenes/Components/NativeBehaviourComponent.h"

#define TRUE_STR "1"
#define FALSE_STR "0"
#define ToFlag(in) (in==1?true:false)

namespace Czuch
{
	bool SerializationComponentHelper::SerializeBaseComponent(Component* component, bool binary)
	{
		if (binary)
		{
			return false;
		}
		else
		{
			SerializerHelper::KeyVal("IsEnabled", component->IsEnabled() ? TRUE_STR : FALSE_STR);
			return true;
		}
	}
	bool Czuch::SerializationComponentHelper::SerializeHeaderComponent(HeaderComponent* component, bool binary)
	{
		if (binary)
		{
			return false;
		}
		else
		{
			SerializerHelper::BeginMap();
			SerializerHelper::KeyVal("Tag", component->GetTag());
			SerializerHelper::KeyVal("Name", component->GetHeader());
			SerializerHelper::EndMap();
			return true;
		}
	}
	bool SerializationComponentHelper::SerializeTransformComponent(TransformComponent* component, bool binary)
	{
		if (binary)
		{
			return false;
		}
		else
		{
			SerializerHelper::BeginMap();
			SerializeBaseComponent(component, binary);
			SerializerHelper::Key("Parent");
			SerializerHelper::ValueGUID(component->GetParentGUID());
			SerializerHelper::Key("Position");
			SerializerHelper::Vec3Val(component->GetLocalPosition());
			SerializerHelper::Value("Rotation");
			SerializerHelper::Vec3Val(component->GetLocalEulerAngles());
			SerializerHelper::Value("Scale");
			SerializerHelper::Vec3Val(component->GetLocalScale());
			SerializerHelper::EndMap();
		}
	}

	bool SerializationComponentHelper::SerializeCameraComponent(CameraComponent* component, bool binary)
	{
		if (binary)
		{
			return false;
		}
		else
		{
			SerializerHelper::BeginMap();
			SerializeBaseComponent(component, binary);
			SerializerHelper::Key("CameraType");
			SerializerHelper::UIntVal((uint32_t)component->GetType());
			SerializeCamera(&component->GetCamera(), binary);
			SerializerHelper::EndMap();
			return true;
		}
	}

	bool SerializationComponentHelper::SerializeGUIDComponent(GUIDComponent* component, bool binary)
	{
		if (binary)
		{
			return false;
		}
		else
		{
			SerializerHelper::BeginMap();
			SerializerHelper::KeyGUIDValue("GUID", component->GetGUID());
			SerializerHelper::EndMap();
			return true;
		}
	}

	bool SerializationComponentHelper::SerializeCamera(Camera* camera, bool binary)
	{
		if (binary)
		{

		}
		else
		{
			SerializerHelper::Key("VerticalFov");
			SerializerHelper::Value(std::to_string(camera->GetFov()));
			SerializerHelper::Key("AspectRatio");
			SerializerHelper::Value(std::to_string(camera->GetAspectRatio()));
			SerializerHelper::Key("NearPlane");
			SerializerHelper::Value(std::to_string(camera->GetNearPlane()));
			SerializerHelper::Key("FarPlane");
			SerializerHelper::Value(std::to_string(camera->GetFarPlane()));
		}
	}


	bool SerializationComponentHelper::DeserializeBaseComponent(Component* component, const YAML::Node& in, bool binary)
	{
		if (binary)
		{
			return false;
		}
		auto isEnabled = in["IsEnabled"].as<int>();
		component->SetEnabled(ToFlag(isEnabled));
		return true;
	}
	bool SerializationComponentHelper::DeserializeHeaderComponent(HeaderComponent* component, const YAML::Node& in, bool binary)
	{
		if (binary)
		{
			return false;
		}

		if (in["Tag"])
		{
			component->SetTag(in["Tag"].as<std::string>());
		}

		if (in["Name"])
		{
			component->SetHeader(in["Name"].as<std::string>());
		}
		return true;
	}

	bool SerializationComponentHelper::DeserializeTransformComponent(TransformComponent* component, const YAML::Node& in, bool binary)
	{
		if (binary)
		{
			return false;
		}

		if (in["Parent"])
		{
			auto parent = in["Parent"].as<uint64_t>();
			component->SetParentFromGUID(GUID(parent));
		}

		if (in["Position"])
		{
			auto pos = in["Position"].as<Vec3>();
			component->SetLocalPosition(pos);
		}

		if (in["Rotation"])
		{
			auto rot = in["Rotation"].as<Vec3>();
			component->SetLocalEulerAngles(rot);
		}

		if (in["Scale"])
		{
			auto scale = in["Scale"].as<Vec3>();
			component->SetLocalScale(scale);
		}

		return true;
	}

	bool SerializationComponentHelper::DeserializeCameraComponent(CameraComponent* component, const YAML::Node& in, bool binary)
	{
		if (binary)
		{
			return false;
		}

		if (in["CameraType"])
		{
			auto type = in["CameraType"].as<uint32_t>();
			component->SetType((CameraType)type);
		}

		if (in["Camera"])
		{
			auto camera = in["Camera"];
			DeserializeCamera(&component->GetCamera(), camera, binary);
		}

		return true;
	}

	bool SerializationComponentHelper::DeserializeGUIDComponent(GUIDComponent* component, const YAML::Node& in, bool binary)
	{
		if (binary)
		{
			return false;
		}
		auto guid = in["GUID"].as<uint64_t>();
		component->SetGUID(GUID(guid));
		return true;
	}

	bool SerializationComponentHelper::DeserializeCamera(Camera* camera, const YAML::Node& in, bool binary)
	{
		if (binary)
		{
			return false;
		}

		if (in["VerticalFov"])
		{
			auto fov = in["VerticalFov"].as<float>();
			camera->SetVerticalFov(fov);
		}

		if (in["AspectRatio"])
		{
			auto aspect = in["AspectRatio"].as<float>();
			camera->SetAspectRatio(aspect);
		}

		if (in["NearPlane"])
		{
			auto nearPlane = in["NearPlane"].as<float>();
			camera->SetNearPlane(nearPlane);
		}

		if (in["FarPlane"])
		{
			auto farPlane = in["FarPlane"].as<float>();
			camera->SetFarPlane(farPlane);
		}
		return true;
	}

}
