#pragma once
#include"ISerializer.h"

namespace Czuch
{
	struct HeaderComponent;
	struct Component;
	struct TransformComponent;

#define Vec3ToString(vec)(std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z));

	class SerializationComponentHelper
	{
	public:
		static bool SerializeBaseComponent(Component* component, bool binary);
		static bool SerializeHeaderComponent(HeaderComponent* component, bool binary);
		static bool SerializeTransformComponent(TransformComponent* component, bool binary);
	};
}


