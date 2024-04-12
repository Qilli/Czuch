#pragma once
#include"Core/TimeDelta.h"
#include"Core/EngineCore.h"
#include"IScene.h"
#include"Entity.h"
#include"Renderer/RenderContext.h"

namespace Czuch
{
	class Renderer;
	class Scene : public IScene
	{
	public:

		Scene(const CzuchStr& sceneName);
		~Scene();

		void OnUpdate(TimeDelta delta);
		void FillRenderContexts(Renderer* renderer);
		Entity CreateEntity(const CzuchStr& entityName,Entity parent=Entity());
		void DestroyEntity(Entity entity);
		Entity GetRootEntity();
		void ClearScene();
		const CzuchStr& GetSceneName() { return m_SceneName; }
		RenderContext& GetGeneralRenderContext() { return m_GeneralRenderContext; }
		RenderContext& GetUIRenderContext() { return m_UIRenderContext; }
		RenderContext& GetDebugRenderContext() { return m_DebugRenderContext; }
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
		RenderContext m_UIRenderContext;
		RenderContext m_DebugRenderContext;
	};
}
