#pragma once
#include"Core/TimeDelta.h"
#include"Core/EngineCore.h"
#include"IScene.h"
#include"Entity.h"
#include"Renderer/RenderContext.h"
#include"Serialization/ISerializer.h"
#include"Core/GUID.h"

namespace Czuch
{
	class Renderer;
	class UIBaseElement;
	class Camera;
	class CZUCH_API Scene : public IScene, public ISerializer
	{
	public:

		Scene(const CzuchStr& sceneName, RenderSettings* settings);
		~Scene();

		void AddUIElement(UIBaseElement* element);

		void OnUpdate(TimeDelta delta);
		void OnFinishFrame();
		void FillRenderContexts(Camera* cam,Renderer* renderer,int width,int height,RenderContextFillParams& fillParams);
		Entity CreateEntity(const CzuchStr& entityName, Entity parent = Entity());
		void DestroyEntity(Entity entity);
		void MarkEntityForDestroy(Entity entity);
		Entity GetRootEntity();
		void ClearScene();
		const CzuchStr& GetSceneName() const { return m_SceneName; }
		RenderContext& GetGeneralRenderContext() { return m_GeneralRenderContext; }
		RenderContext& GetDebugRenderContext() { return m_DebugRenderContext; }
		entt::registry& GetRegistry() override { return m_Registry; }
	public:
		std::vector<UIBaseElement*>& GetSceneUIElements() { return m_UIElements; }
		void ForEachEntity(std::function<void(Entity)> func);
		void ForEachEntityWithHierarchy(std::function<void(Entity)> func);
		entt::entity GetEntityWithGUID(GUID guid) override;
		Entity GetEntityObjectWithGUID(GUID guid);
	public:
		CameraComponent* FindPrimaryCamera() override;
		CameraComponent* FindEditorCamera() override;
		void SetPrimaryCamera(CameraComponent* camera) override;
		void SetEditorCamera(CameraComponent* camera) override;
	public:
		void OnAttached() {}
		void OnDettached() {}
	public:
		virtual bool Serialize(YAML::Emitter& out, bool binary = false) override;
		virtual bool Deserialize(const YAML::Node& in, bool binary = false) override;
	private:
		void CreateRenderContexts();
		void DestroyMarkedEntities();
		void GetCurrentActiveCamera();
	private:
		friend class Entity;
		friend class SceneSerializer;
		Camera* m_CurrentFrameCamera;
		entt::registry m_Registry;
		Entity m_RootEntity;
		CzuchStr m_SceneName;
		RenderContext m_GeneralRenderContext;
		RenderContext m_DebugRenderContext;
		std::vector<UIBaseElement*> m_UIElements;
		std::vector<Entity> m_EntitiesToDestroy;
		RenderSettings* m_RenderSettings;
	};
}
