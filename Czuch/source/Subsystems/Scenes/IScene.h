#pragma once
#include"entt/entt.hpp"
#include"Serialization/ISerializer.h"

namespace Czuch
{
	class CameraComponent;
	class IScene: public ISerializer
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
		virtual CameraComponent* FindEditorCamera() = 0;
		virtual void SetPrimaryCamera(CameraComponent* camera) = 0;
		virtual void SetEditorCamera(CameraComponent* camera) = 0;
		bool IsDirty() const { return m_isDirty; }
		void Dirty() { m_isDirty = true; }
		void SetDirty(bool dirty) { m_isDirty = dirty; }
		virtual entt::entity GetEntityWithGUID(GUID guid)=0;
		friend class Entity;
	protected:
		virtual entt::registry& GetRegistry() = 0;
	protected:
		bool m_isDirty = false;
	};
}