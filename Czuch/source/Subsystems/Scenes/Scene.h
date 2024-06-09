#pragma once
#include"Core/TimeDelta.h"
#include"Core/EngineCore.h"
#include"IScene.h"
#include"Entity.h"
#include"Renderer/RenderContext.h"

namespace Czuch
{
	class Renderer;
	class UIBaseElement;
	class Camera;
	class CZUCH_API Scene : public IScene
	{
	public:

		Scene(const CzuchStr& sceneName);
		~Scene();

		void AddUIElement(UIBaseElement* element);

		void OnUpdate(TimeDelta delta);
		void FillRenderContexts(Camera* cam,Renderer* renderer,int width,int height);
		Entity CreateEntity(const CzuchStr& entityName, Entity parent = Entity());
		void DestroyEntity(Entity entity);
		Entity GetRootEntity();
		void ClearScene();
		const CzuchStr& GetSceneName() { return m_SceneName; }
		RenderContext& GetGeneralRenderContext() { return m_GeneralRenderContext; }
		RenderContext& GetDebugRenderContext() { return m_DebugRenderContext; }
		entt::registry& GetRegistry() override { return m_Registry; }
	public:
		std::vector<UIBaseElement*>& GetSceneUIElements() { return m_UIElements; }
	public:
		CameraComponent* FindPrimaryCamera() override;
	public:
		void OnAttached() {}
		void OnDettached() {}
	private:
		void CreateRenderContexts();
	private:
		friend class Entity;
		entt::registry m_Registry;
		Entity m_RootEntity;
		Entity m_MainCameraEntity;
		CzuchStr m_SceneName;
		RenderContext m_GeneralRenderContext;
		RenderContext m_DebugRenderContext;
		std::vector<UIBaseElement*> m_UIElements;
	};
}
