#pragma once
#include"entt/entt.hpp"
#include"Serialization/ISerializer.h"

namespace Czuch
{
	class CameraComponent;
	class Entity;
	struct Camera;
	struct FrameGraphControl;
	struct LightObjectInfo;
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
		virtual CameraComponent* GetPrimaryCamera() = 0;
		virtual CameraComponent* FindEditorCamera() = 0;
		virtual void SetPrimaryCamera(CameraComponent* camera) = 0;
		virtual void SetEditorCamera(CameraComponent* camera) = 0;
		virtual void CameraEnabledChanged(CameraComponent* camera) = 0;
		virtual void CameraAdded(CameraComponent* camera) = 0;
		virtual void CameraRemoved(CameraComponent* camera) = 0;
		virtual FrameGraphControl* GetFrameGraphControl(Camera* camera) = 0;
		virtual FrameGraphControl* GetFrameGraphControl(int index) = 0;
		bool IsDirty() const { return m_isDirty; }
		void Dirty() { m_isDirty = true; }
		void SetDirty(bool dirty) { m_isDirty = dirty; }
		virtual entt::entity GetEntityWithGUID(GUID guid)=0;
		virtual const Color GetAmbientColor() const = 0;
		virtual const Array<LightObjectInfo>& GetAllLightObjects() const=0;
		virtual void ForEachEntity(std::function<void(Entity*)> func) = 0;
		virtual void ForEachEntityWithHierarchy(std::function<void(Entity*)> func) =0;
		friend class Entity;
	protected:
		virtual entt::registry& GetRegistry() = 0;
	protected:
		bool m_isDirty = false;
	};
}