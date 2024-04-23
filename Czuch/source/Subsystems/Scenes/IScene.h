#pragma once
#include"entt.hpp"

namespace Czuch
{
	class CameraComponent;
	class IScene
	{
	public:
		template<typename T>
		entt::entity FindEntityWithComponent()
		{
			auto view = GetRegistry().view<T>();
			for (auto entity : view)
			{
				return entity;
			}
			return entt::null;
		}
		virtual CameraComponent* FindPrimaryCamera() = 0;
		friend class Entity;
	protected:
		virtual entt::registry& GetRegistry() = 0;
	};
}