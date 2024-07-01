#include "czpch.h"
#include "SerializationComponentHelper.h"
#include"../Subsystems/Scenes/Components/TransformComponent.h"
#include"../Subsystems/Scenes/Components/HeaderComponent.h"
#include"../Subsystems/Scenes/Components/CameraComponent.h"
#include"../Subsystems/Scenes/Components/MeshComponent.h"
#include"../Subsystems/Scenes/Components/MeshRendererComponent.h"
#include"../Subsystems/Scenes/Components/NativeBehaviourComponent.h"

#define TRUE "1"
#define FALSE "0"

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
			SerializerHelper::KeyVal("IsEnabled", component->IsEnabled() ? TRUE : FALSE);
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
			SerializeBaseComponent(component, binary);
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
			std::string pos = Vec3ToString(component->GetLocalPosition());
			SerializerHelper::KeyVal("Position", pos);
			std::string rot = Vec3ToString(component->GetLocalEulerAngles());
			SerializerHelper::KeyVal("Rotation", rot);
			std::string scale = Vec3ToString(component->GetLocalScale());
			SerializerHelper::KeyVal("Scale", scale);
			SerializerHelper::EndMap();
		}
	}
}

