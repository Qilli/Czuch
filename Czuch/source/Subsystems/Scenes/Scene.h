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

		Scene(const CzuchStr& sceneName,GraphicsDevice* device);
		~Scene();

		void AddUIElement(UIBaseElement* element);

		void OnUpdate(TimeDelta delta);
		void OnFinishFrame();
		/// <summary>
		/// Fill render context for selected camera and rendering pass
		/// Some contexts can be shared between different rendering pass like preZ pass and opaque lighting pass
		/// </summary>
		/// <param name="cam"></param>
		/// <param name="renderer"></param>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <param name="fillParams"></param>
		/// <returns></returns>
		RenderContext* FillRenderContexts(Camera* cam, Renderer* renderer, int width, int height, RenderContextFillParams& fillParams);
		/// <summary>
		///  Gather all debug elements do draw from all the entities in the scene
		///  (lines, triangles, points, simple meshes)
		/// </summary>
		/// <param name="cam"></param>
		/// <param name="renderer"></param>
		/// <param name="fillParams"></param>
		void FillDebugDrawElements(Camera* cam, Renderer* renderer,RenderContextFillParams& fillParams);
		void OnPostRender(Camera* camera, RenderContextFillParams* fillParams);
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
		void OnSceneActive(Renderer* renderer,GraphicsDevice* device);
		/// <summary>
		/// Event called from renderer when window is resized or target texture is resized(for editor width/height will be equal to render view port), only to active scene
		/// </summary>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <param name="windowSizeChanged">if true then the entire window size changed, otherwise only render view</param>
		void OnResize(U32 width, U32 height,bool windowSizeChanged);
		/// <summary>
		/// This method is called when we are about to start rendering next frame, after we aquire the next image from swapchain
		/// </summary>
		void BeforeFrameGraphExecute(CommandBufferHandle cmdBuffer,U32 currentFrame, DeletionQueue& deletionQueue);
		/// <summary>
		/// Here we want to make all render contexts dirty so we can update them at the beginning of the next frame
		/// </summary>
		void AfterFrameGraphExecute(CommandBufferHandle cmdBuffer);
		/// <summary>
		/// Get scene data buffers for selected camera, if null then use primary camera
		/// </summary>
		/// <param name="camera"></param>
		/// <param name="frame"></param>
		/// <param name="renderPassType"></param>
		/// <returns></returns>
		SceneDataBuffers GetSceneDataBuffers(Camera* camera, U32 frame, RenderPassType renderPassType);
	public:
		std::vector<UIBaseElement*>& GetSceneUIElements() { return m_UIElements; }
		void ForEachEntity(std::function<void(Entity*)> func) override;
		void ForEachEntityWithHierarchy(std::function<void(Entity*)> func) override;
		entt::entity GetEntityWithGUID(GUID guid) override;
		Entity GetEntityObjectWithGUID(GUID guid);
	public:
		CameraComponent* GetPrimaryCamera() override;
		CameraComponent* FindEditorCamera() override;
		CameraComponent* FindPrimaryCamera();
		void SetPrimaryCamera(CameraComponent* camera) override;
		void SetEditorCamera(CameraComponent* camera) override;
		void CameraEnabledChanged(CameraComponent* camera) override;
		void CameraAdded(CameraComponent* camera) override;
		void CameraRemoved(CameraComponent* camera) override;
		Array<SceneCameraControl>& GetCamerasControl() { return m_CamerasControl; }
		RenderContext* GetRenderContext(RenderPassType type, Camera* camera);
	public:
		void OnAttached() { CheckAndAddStartCamera(); m_isDirty = false; UpdateAllCameras(); }
		void OnDettached();
	public:
		virtual bool Serialize(YAML::Emitter& out, bool binary = false) override;
		virtual bool Deserialize(const YAML::Node& in, bool binary = false) override;
	public:
		void SetClearColor(const Color& color) { m_ClearColor = color; }
		const Color GetClearColor() const { return m_ClearColor; }
		void SetAmbientColor(const Color& color) { m_AmbientColor = color; }
		const Color GetAmbientColor() const { return GammaToLinear(m_AmbientColor); }
	public:
		FrameGraphControl* GetFrameGraphControl(Camera* camera) override;
		FrameGraphControl* GetFrameGraphControl(int index) override;
		void OnResizeRenderPassType(RenderPassType type, int width, int height);
		void InitFrameGraphsControls();
		int GetActiveFrameGraphsCount() const { return m_CamerasControl.size(); }
		void ForEachFrameGraph(std::function<void(FrameGraphControl*)> func);
	private:
		void DestroyMarkedEntities();
		void CheckAndAddStartCamera();
		void UpdateAllCameras();
	private:
		friend class Entity;
		friend class SceneSerializer;
		Camera* m_CurrentFrameCamera;
		entt::registry m_Registry;
		Entity m_RootEntity;
		CzuchStr m_SceneName;
		Array<UIBaseElement*> m_UIElements;
		Array<Entity> m_EntitiesToDestroy;
		Array<SceneCameraControl> m_CamerasControl;
		RenderObjectsContainer m_RenderObjects;
		GraphicsDevice* m_Device;
		Renderer* m_Renderer;
		Color m_ClearColor;
		Color m_AmbientColor;
	};
}
