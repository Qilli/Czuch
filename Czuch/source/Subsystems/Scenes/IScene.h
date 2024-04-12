#pragma once
#include"entt.hpp"

namespace Czuch
{
	class IScene
	{
		friend class Entity;
	protected:
		virtual entt::registry GetRegistry() = 0;
	};
}