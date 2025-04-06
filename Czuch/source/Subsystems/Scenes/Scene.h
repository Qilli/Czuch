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
		void FillRenderContexts(Camera* cam,Renderer* renderer,int width,int height,RenderContextFillParams& fillParams,RenderContext* renderContext);
		Entity CreateEntity(const CzuchStr& entityName, Entity parent = Entity());
		Entity AddModelToScene(Czuch::AssetHandle model, const CzuchStr& entityName, Entity parent = Entity());
		void DestroyEntity(Entity entity);
		void MarkEntityForDestroy(Entity entity);
		Entity GetRootEntity();
		void ClearScene();
		const CzuchStr& GetSceneName() const { return m_SceneName; }
		entt::registry& GetRegistry() override { return m_Registry; }
		RenderObjectsContainer& GetRenderObjects() { return m_RenderObjects; }
		const Array<LightObjectInfo>& GetAllLightObjects() const { return m_RenderObjects.allLights; }
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
		void OnAttached() { CheckAndAddStartCamera(); GetCurrentActiveCamera(); }
		void OnDettached() {}
	public:
		virtual bool Serialize(YAML::Emitter& out, bool binary = false) override;
		virtual bool Deserialize(const YAML::Node& in, bool binary = false) override;
	public:
		void SetClearColor(const Color& color) { m_ClearColor = color; }
		const Color GetClearColor() const { return m_ClearColor; }
		void SetAmbientColor(const Color& color) { m_AmbientColor = color; }
		const Color GetAmbientColor() const { return m_AmbientColor; }
		RenderSettings* GetRenderSettings() { return m_RenderSettings; }
	private:
		void DestroyMarkedEntities();
		void GetCurrentActiveCamera();
		void CheckAndAddStartCamera();
	private:
		friend class Entity;
		friend class SceneSerializer;
		Camera* m_CurrentFrameCamera;
		entt::registry m_Registry;
		Entity m_RootEntity;
		CzuchStr m_SceneName;
		Array<UIBaseElement*> m_UIElements;
		Array<Entity> m_EntitiesToDestroy;
		RenderObjectsContainer m_RenderObjects;
		RenderSettings* m_RenderSettings;
		Color m_ClearColor;
		Color m_AmbientColor;
	};
}
