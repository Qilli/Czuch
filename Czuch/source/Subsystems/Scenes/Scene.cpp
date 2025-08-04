#include"czpch.h"
#include"Scene.h"
#include"Entity.h"
#include"Components/HeaderComponent.h"
#include"Components/TransformComponent.h"
#include"Components/MeshComponent.h"
#include"Components/MeshRendererComponent.h"
#include"Components/CameraComponent.h"
#include"Components/NativeBehaviourComponent.h"	
#include"Subsystems/UI/UIBaseElement.h"
#include"Subsystems/Scenes/Components/CameraComponent.h"
#include"Subsystems/Scenes/Components/LightComponent.h"
#include"Subsystems/Scenes/NativeBehaviours/NativeFree3DCameraController.h"
#include"Renderer/Renderer.h"
#include"Renderer/GraphicsDevice.h"
#include"Subsystems/Assets/AssetsManager.h"
#include"Subsystems/Assets/Asset/ModelAsset.h"


namespace Czuch
{
	const CzuchStr SceneTag = "SceneControl";

	Scene::Scene(const CzuchStr& sceneName,GraphicsDevice* device) : m_SceneName(sceneName), m_Device(device),m_Renderer(nullptr)
	{
		m_RootEntity = Entity{ m_Registry.create(),this };
		m_RootEntity.AddComponent<HeaderComponent>(sceneName, "Root", Layer{ 0 });
		m_RootEntity.AddComponent<TransformComponent>();
		m_RootEntity.AddComponent<ActiveComponent>();
		m_RootEntity.AddComponent<GUIDComponent>(GUID());

		m_AmbientColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
		m_ClearColor = Color(0.0f, 0.0f, 0.0f, 1.0f);

		//create default game mode camera
		Entity cameraEntity = CreateEntity("MainCamera", m_RootEntity);
		auto& cam = cameraEntity.AddComponent<CameraComponent>();
		cam.SetAsPrimary();
		cam.SetType(CameraType::GameCamera);
		cameraEntity.GetComponent<TransformComponent>().SetLocalPosition({ 0.0f,0.0f,3.0f });

		CheckAndAddStartCamera();
		Dirty();
	}


	Scene::~Scene()
	{
		m_Registry.clear();
	}

	void Scene::AddUIElement(UIBaseElement* element)
	{
		m_UIElements.push_back(element);
		Dirty();
	}

	void Scene::OnUpdate(TimeDelta delta)
	{
		if (IsDirty())
		{
			UpdateAllCameras();
			m_isDirty = false;
		}

		auto view = m_Registry.view<NativeBehaviourComponent, ActiveComponent>();
		for (auto entity : view)
		{
			auto& nativeBehaviour = view.get<NativeBehaviourComponent>(entity);
			nativeBehaviour.OnUpdate(delta);
		}

		//fill render list
		m_RenderObjects.allObjects.clear();
		auto renderableView = m_Registry.view<TransformComponent, MeshComponent, MeshRendererComponent, ActiveComponent>(entt::exclude<DestroyedComponent>);
		for (auto entity : renderableView)
		{

			auto& transform = renderableView.get<TransformComponent>(entity);
			auto& mesh = renderableView.get<MeshComponent>(entity);
			auto& meshRenderer = renderableView.get<MeshRendererComponent>(entity);

			m_RenderObjects.allObjects.push_back(RenderObjectInfo{ .transform = &transform,.mesh = &mesh,.meshRenderer = &meshRenderer });	
		}

		//fill lights list
		m_RenderObjects.allLights.clear();


		//[TODO] Get all lights from the scene visible in the camera(collision check)
		auto lightView = m_Registry.view<LightComponent,TransformComponent, ActiveComponent>(entt::exclude<DestroyedComponent>);
		for (auto entity : lightView)
		{
			auto& light = lightView.get<LightComponent>(entity);
			auto& transform = lightView.get<TransformComponent>(entity);
			auto position = std::move(transform.GetWorldPosition());
			auto direction = -transform.GetWorldForward();
			m_RenderObjects.allLights.push_back({ {.positionWithType=Vec4(position.x,position.y,position.z,light.GetLightType()),
				.color=light.GetColor(),
				.directionWithRange=Vec4(direction.x,direction.y,direction.z,light.GetLightRange()),
				.spotInnerOuterAngle=Vec4(light.GetInnerAngleCos(),light.GetOuterAngleCos(),0,0)},& transform,& light});
		}


		//fill all cameras with visible objects
		for (auto& cameraControl : m_CamerasControl)
		{
			cameraControl.UpdateVisibleObjects(m_RenderObjects);
		}
	}

	void Scene::OnFinishFrame()
	{
		DestroyMarkedEntities();
		auto view = m_Registry.view<NativeBehaviourComponent, ActiveComponent>();
		for (auto entity : view)
		{
			auto& nativeBehaviour = view.get<NativeBehaviourComponent>(entity);
			nativeBehaviour.OnFinishFrame();
		}
		for (auto uiElement : m_UIElements)
		{
			uiElement->OnFinishFrame();
		}
	}

	RenderContext* Scene::FillRenderContexts(Camera* cam, Renderer* renderer, int width, int height, RenderContextFillParams& fillParams)
	{
		//get main camera
		Camera* currentCamera = cam;
		if (cam == nullptr)
		{
			currentCamera = m_CurrentFrameCamera;
		}

		currentCamera->SetAspectRatio((float)width / (float)height);
		for (auto& cameraControl : m_CamerasControl)
		{
			if (cameraControl.camera == currentCamera)
			{
				RenderContext* context= cameraControl.FillRenderList(renderer->GetDevice(),fillParams);
				return context;
			}
		}
		LOG_BE_ERROR("[Scene] Scene does not have camera control for the camera in FillRenderContexts.");
		return nullptr;
	}

	void Scene::AfterFrameGraphExecute(CommandBufferHandle cmdBuffer)
	{
		for (auto& cameraControl : m_CamerasControl)
		{
			cameraControl.frameGraphControl.AfterFrameGraphExecute(m_Device->AccessCommandBuffer(cmdBuffer));
		}
	}

	void Scene::FillDebugDrawElements(Camera* cam, Renderer* renderer, RenderContextFillParams& fillParams)
	{
		Camera* currentCamera = cam;
		if (cam == nullptr)
		{
			currentCamera = m_CurrentFrameCamera;
		}

		for (auto& cameraControl : m_CamerasControl)
		{
			if (cameraControl.camera == currentCamera)
			{
				cameraControl.FillDebugDrawElements(renderer->GetDevice(), fillParams);
				return;
			}
		}
		LOG_BE_ERROR("[Scene] Scene does not have camera control for the camera in FillDebugDrawElements.");
		return;
	}

	void Scene::OnPostRender(Camera* camera, RenderContextFillParams* fillParams)
	{
		Camera* currentCamera = camera;
		if (camera == nullptr)
		{
			currentCamera = m_CurrentFrameCamera;
		}

		for (auto& cameraControl : m_CamerasControl)
		{
			if (cameraControl.camera == currentCamera)
			{
				cameraControl.OnPostRender();
				return;
			}
		}
		LOG_BE_ERROR("[Scene] Scene does not have camera control for the camera in OnPostRender.");
		return;
	}

	Entity Scene::CreateEntity(const CzuchStr& entityName, Entity parent)
	{
		Entity entity = { m_Registry.create(),this };

		if (!m_RootEntity)
		{
			m_RootEntity = entity;
		}

		CzuchStr tag = "Default";
		entity.AddComponent<HeaderComponent>(entityName, tag, Layer{ 0 });
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<ActiveComponent>();
		entity.AddComponent<GUIDComponent>(GUID());


		if (parent.IsValid())
		{
			parent.AddChild(entity);
		}
		else
		{
			entity.SetAsParent(m_RootEntity);
		}
		Dirty();
		return entity;
	}

	Entity Scene::AddModelToScene(Czuch::AssetHandle model, const CzuchStr& entityName, Entity parent)
	{
		auto modelAsset = AssetsManager::Get().GetAsset<ModelAsset>(model);
		if (!modelAsset)
		{
			LOG_BE_ERROR("Model asset is null, could not add it to scene.");
			return Entity();
		}
		Entity root=modelAsset->AddModelToScene(this, parent);
		root.GetComponent<HeaderComponent>().SetHeader(entityName);
		return root;
	}


	template<typename T>
	void TryToRemoveComponentFromEntity(Entity entity)
	{
		if (entity.HasComponent<T>())
		{
			auto& comp = entity.GetComponent<T>();
			comp.OnRemoved();
			entity.RemoveComponent<T>();
		}
	}

	void Scene::DestroyEntity(Entity entity)
	{
		auto internalHandle = entity.GetInternalHandle();

		//clean up all children and remove parent
		auto& transform = entity.GetComponent<TransformComponent>();
		for (auto child : transform.GetChildren())
		{
			DestroyEntity(child);
		}

		transform.SetParent(Entity::CreateInvalidEntity());

		//manually test if entitt has a component and remove it
		TryToRemoveComponentFromEntity<HeaderComponent>(entity);
		TryToRemoveComponentFromEntity<TransformComponent>(entity);
		TryToRemoveComponentFromEntity<ActiveComponent>(entity);
		TryToRemoveComponentFromEntity<MeshComponent>(entity);
		TryToRemoveComponentFromEntity<MeshRendererComponent>(entity);
		TryToRemoveComponentFromEntity<CameraComponent>(entity);
		TryToRemoveComponentFromEntity<NativeBehaviourComponent>(entity);

		m_Registry.destroy(internalHandle);
		Dirty();
	}

	void Scene::MarkEntityForDestroy(Entity entity)
	{
		m_EntitiesToDestroy.push_back(entity);
	}

	Entity Scene::GetRootEntity()
	{
		return m_RootEntity;
	}

	void Scene::ClearScene()
	{
		OnDettached();
		m_Registry.clear();
		m_RootEntity = Entity{ m_Registry.create(),this };
		m_RootEntity.AddComponent<HeaderComponent>(m_SceneName, "Root", Layer{ 0 });
		m_RootEntity.AddComponent<TransformComponent>();
		m_RootEntity.AddComponent<ActiveComponent>();
		m_RootEntity.AddComponent<GUIDComponent>(GUID());
	}

	void Scene::OnSceneActive(Renderer* renderer,GraphicsDevice* device)
	{
		m_Device = device;
		m_Renderer = renderer;
		for (auto& cameraControl : m_CamerasControl)
		{
			cameraControl.OnSceneActive(renderer,m_Device,this);
		}
		EngineRoot::Get().TryBuildDefaultAssets();

		for (auto& cameraControl : m_CamerasControl)
		{
			cameraControl.AfterSceneActive();
		}
	}

	void Scene::OnResize(U32 width, U32 height,bool windowSizeChanged)
	{

		if (EngineRoot::GetEngineSettings().engineMode == EngineMode::Editor && windowSizeChanged)
		{
			return;
		}

		for (auto& cameraControl : m_CamerasControl)
		{
			cameraControl.OnResize(m_Device, width, height,windowSizeChanged);
		}
	}

	void Scene::BeforeFrameGraphExecute(CommandBufferHandle cmdBuffer,U32 currentFrame, DeletionQueue& deletionQueue)
	{
		for (auto& cameraControl : m_CamerasControl)
		{
			cameraControl.UpdateSceneDataBuffers(m_Device,currentFrame, deletionQueue);
			cameraControl.frameGraphControl.BeforeFrameGraphExecute(m_Device->AccessCommandBuffer(cmdBuffer));
		}
	}

	SceneDataBuffers Scene::GetSceneDataBuffers(Camera* camera, U32 frame, RenderPassType renderPassType)
	{
		if (camera == nullptr)
		{
			camera = m_CurrentFrameCamera;
		}

		for (auto& control : m_CamerasControl)
		{
			if (control.camera == camera)
			{
				return control.GetSceneDataBuffers(frame);
			}
		}
		return m_CamerasControl[0].GetSceneDataBuffers(frame);
	}

	void Scene::ForEachEntity(std::function<void(Entity*)> func)
	{
		auto view = m_Registry.view<HeaderComponent>();
		for (auto entity : view)
		{
			auto current = Entity{ entity,this };
			func(&current);
		}
	}

	void ForEachChildEntity(TransformComponent* transform, std::function<void(Entity*)> func, Czuch::IScene* scene)
	{
		for (auto child : transform->GetChildren())
		{
			if (child.IsDestroyed())
			{
				continue;
			}
			func(&child);
			ForEachChildEntity(&child.GetComponent<TransformComponent>(), func, scene);
		}
	}

	void Scene::ForEachEntityWithHierarchy(std::function<void(Entity*)> func)
	{
		TransformComponent* rootTransform = &m_RootEntity.GetComponent<TransformComponent>();
		func(&m_RootEntity);
		ForEachChildEntity(rootTransform, func, this);
	}

	entt::entity Scene::GetEntityWithGUID(GUID guid)
	{
		auto guids = m_Registry.view<GUIDComponent>(entt::exclude<DestroyedComponent>);
		for (auto entity : guids)
		{
			auto& guidComponent = guids.get<GUIDComponent>(entity);
			if (guidComponent.GetGUID() == guid)
			{
				return entity;
			}
		}
	}

	Entity Scene::GetEntityObjectWithGUID(GUID guid)
	{
		auto entity = GetEntityWithGUID(guid);
		if (entity != entt::null)
		{
			return Entity{ entity,this };
		}
	}

	CameraComponent* Scene::GetPrimaryCamera()
	{
		auto view = m_Registry.view<CameraComponent>(entt::exclude<DestroyedComponent>);
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);

			if (EngineRoot::GetEngineSettings().engineMode == EngineMode::Runtime && camera.IsPrimary() && camera.IsEnabled())
			{
				return &camera;
			}
			else if (EngineRoot::GetEngineSettings().engineMode == EngineMode::Editor && camera.GetType() == CameraType::EditorCamera && camera.IsEnabled())
			{
				return &camera;
			}

		}
		LOG_BE_ERROR("{0} Scene does not have primary camera", SceneTag);
		return nullptr;
	}

	CameraComponent* Scene::FindEditorCamera()
	{
		auto view = m_Registry.view<CameraComponent>(entt::exclude<DestroyedComponent>);
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			if (camera.GetType() == CameraType::EditorCamera)
			{
				return &camera;
			}
		}
		LOG_BE_ERROR("{0} Scene does not have editor camera", SceneTag);
		return nullptr;
	}

	CameraComponent* Scene::FindPrimaryCamera()
	{
		auto view = m_Registry.view<CameraComponent>(entt::exclude<DestroyedComponent>);
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			if (EngineRoot::GetEngineSettings().engineMode == EngineMode::Runtime && camera.GetType() == CameraType::GameCamera && camera.IsEnabled())
			{
				return &camera;
			}
			else if (EngineRoot::GetEngineSettings().engineMode == EngineMode::Editor && camera.GetType() == CameraType::EditorCamera && camera.IsEnabled())
			{
				return &camera;
			}
		}
		return nullptr;
	}

	void Scene::SetPrimaryCamera(CameraComponent* camera)
	{
		auto view = m_Registry.view<CameraComponent>(entt::exclude<DestroyedComponent>);
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			camera.SetPrimaryFlag(false);
		}
		camera->SetPrimaryFlag(true);
		m_CurrentFrameCamera = &camera->GetCamera();
		Dirty();
	}

	void Scene::SetEditorCamera(CameraComponent* camera)
	{
		auto view = m_Registry.view<CameraComponent>(entt::exclude<DestroyedComponent>);
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			camera.SetType(CameraType::GameCamera);
		}
		camera->SetType(CameraType::EditorCamera);
		Dirty();
	}

	void Scene::CameraEnabledChanged(CameraComponent* camera)
	{
		Dirty();
	}

	void Scene::CameraAdded(CameraComponent* camera)
	{
		Dirty();
	}

	void Scene::CameraRemoved(CameraComponent* camera)
	{
		Dirty();
	}

	RenderContext* Scene::GetRenderContext(RenderPassType type, Camera* camera)
	{
		if (camera == nullptr)
		{
			camera = m_CurrentFrameCamera;
		}

		for (auto& cameraControl : m_CamerasControl)
		{
			if (cameraControl.camera == camera)
			{
				return cameraControl.GetRenderContext(type);
			}
		}
		LOG_BE_ERROR("[Scene] Scene does not have camera control for the camera in GerRenderContext.");
		return nullptr;
	}

	FrameGraphControl* Scene::GetFrameGraphControl(Camera* camera)
	{
		if (camera == nullptr)
		{
			camera = m_CurrentFrameCamera;
		}

		for (auto& cameraControl : m_CamerasControl)
		{
			if (cameraControl.camera == camera)
			{
				return &cameraControl.frameGraphControl;
			}
		}
	}

	FrameGraphControl* Scene::GetFrameGraphControl(int index)
	{
		CZUCH_BE_ASSERT(index >= 0 && index < m_CamerasControl.size(), "Index out of range in GetFrameGraphControl");
		return &m_CamerasControl[index].frameGraphControl;
	}

	void Scene::OnResizeRenderPassType(RenderPassType type, int width, int height)
	{
		for (auto& cameraControl : m_CamerasControl)
		{
			cameraControl.frameGraphControl.ResizeRenderPassType(type, width, height);
		}
	}

	void Scene::InitFrameGraphsControls()
	{
		for (auto& cameraControl : m_CamerasControl)
		{
			cameraControl.frameGraphControl.Init();
		}
	}

	void Scene::ForEachFrameGraph(std::function<void(FrameGraphControl*)> func)
	{
		for (auto& cameraControl : m_CamerasControl)
		{
			func(&cameraControl.frameGraphControl);
		}

	}

	void Scene::DestroyMarkedEntities()
	{
		for (auto entity : m_EntitiesToDestroy)
		{
			DestroyEntity(entity);
		}
		m_EntitiesToDestroy.clear();
	}

	void Scene::CheckAndAddStartCamera()
	{
		if (EngineRoot::Get().GetGameMode() == EngineMode::Editor)
		{
			//create default editor camera
			Entity editorCameraEntity = CreateEntity("EditorCamera", m_RootEntity);
			auto& editorCam = editorCameraEntity.AddComponent<CameraComponent>();
			editorCam.SetType(CameraType::EditorCamera);
			editorCameraEntity.GetComponent<TransformComponent>().SetLocalPosition({ 0.0f,0.0f,3.0f });
			auto& headerComponent = editorCameraEntity.GetComponent<HeaderComponent>();
			headerComponent.SetTag("EditorCamera");
			headerComponent.SetVisibleInEditorHierarchy(false);

			//add 3d control component
			auto& nativeScriptsComponent = editorCameraEntity.AddComponent<NativeBehaviourComponent>();
			auto& behaviour = nativeScriptsComponent.AddNativeBehaviour<NativeFree3DCameraController>();
			behaviour.SetRunningMode(ScriptRunningMode::EditorOnly);
		}
	}

	void Scene::UpdateAllCameras()
	{
		m_isDirty = false;
		auto cam = GetPrimaryCamera();

		if (cam!=nullptr)
		{
			m_CurrentFrameCamera = &cam->GetCamera();
		}
		else
		{
			auto cameraComp = FindPrimaryCamera();
			if (cameraComp != nullptr)
			{
				m_CurrentFrameCamera = &cameraComp->GetCamera();
			}
			else
			{
				LOG_BE_ERROR("{0} Scene does not have primary camera", SceneTag);
				return;
			}
		}

		//cameras changed, but we need to check if we really need to recreate anything or just add new camera control
		auto view = m_Registry.view<CameraComponent>(entt::exclude<DestroyedComponent>);

		//we need some temp buffers to create a diff
		Array<CameraComponent*> camerasToAdd;
		camerasToAdd.reserve(10);
		Array<Camera*> camerasToRemove;
		camerasToRemove.reserve(10);
		U32 camerasCount = 0;

		//fill diffs
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			camerasCount++;

			bool found = false;
			for (auto& cameraControl : m_CamerasControl)
			{
				if (cameraControl.camera == &camera.GetCamera())
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				camerasToAdd.push_back(&camera);
			}
		}

		//check if we have cameras to remove
		//first check camera count, if camera counts == m_CamerasControl.size() + camerasToAdd.size() then we don't need to remove anything
		if (camerasCount != m_CamerasControl.size() + camerasToAdd.size())
		{
			for (auto& cameraControl : m_CamerasControl)
			{
				bool found = false;
				for (auto entity : view)
				{
					auto& camera = view.get<CameraComponent>(entity);
					if (cameraControl.camera == &camera.GetCamera())
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					camerasToRemove.push_back(cameraControl.camera);
				}
			}
		}

		//if we have cameras to remove, we need to remove them
		if (camerasToRemove.size() > 0)
		{
			for (auto& camera : camerasToRemove)
			{
				for (auto it = m_CamerasControl.begin(); it != m_CamerasControl.end();)
				{
					if (it->camera == camera)
					{
						it->Release(m_Device);
						it = m_CamerasControl.erase(it);
					}
					else
					{
						++it;
					}
				}
			}
		}

		//if we have cameras to add, we need to add them
		if (camerasToAdd.size() > 0)
		{
			for (auto& camera : camerasToAdd)
			{
				SceneCameraControl cameraControl;
				cameraControl.camera = &camera->GetCamera();
				cameraControl.isPrimaryCamera = camera->IsPrimary();
				cameraControl.OnSceneActive(m_Renderer, m_Device, this);
				cameraControl.AfterSceneActive();
				if (m_Renderer != nullptr)
				{
					U32 debugFlags = m_Renderer->GetDebugRenderingFlags();
					cameraControl.frameGraphControl.SetDebugRenderFlagsGroup(debugFlags);
				}
				m_CamerasControl.push_back(std::move(cameraControl));
			}
		}

		//make sure primary camera is first in the list
		//this sort should use move semantic! should.. [todo] profile it.
		//dodac profile pamiêci tutaj
		std::sort(m_CamerasControl.begin(), m_CamerasControl.end(), [](const SceneCameraControl& a, const SceneCameraControl& b)
			{
				return a.isPrimaryCamera > b.isPrimaryCamera;
			});

	}

	void Scene::OnDettached()
	{
		for (auto& cameraControl : m_CamerasControl)
		{
			cameraControl.Release(m_Device);
		}
		m_CamerasControl.clear();
	}

#pragma region Serialization
	bool Scene::Serialize(YAML::Emitter& out, bool binary)
	{
		if (binary)
		{
			return true;
		}
		SerializerHelper::SetEmitter(&out);
		SerializerHelper::BeginMap();
		SerializerHelper::KeyVal("Scene", GetSceneName());
		SerializerHelper::Key("Entities");
		SerializerHelper::BeginSeq();
		ForEachEntityWithHierarchy([&](Entity* entity)
			{
				if (entity->GetComponent<HeaderComponent>().GetTag() == "EditorCamera")
				{
					return true;
				}

				if (entity->IsValid())
				{
					bool result = entity->Serialize(out, binary);
					if (result == false)
					{
						LOG_BE_ERROR("[Scene][Save]Failed to save scene {0} to the file. Entity {1} serialization failed.", GetSceneName(), entity->GetComponent<HeaderComponent>().GetHeader());
						return false;
					}
				}
				else
				{
					LOG_BE_ERROR("[Scene][Save]Failed to save scene {0} to the file. Entity is invalid.", GetSceneName());
					return false;
				}
			});
		SerializerHelper::EndSeq();
		SerializerHelper::EndMap();
		return true;
	}

	bool Scene::Deserialize(const YAML::Node& in, bool binary)
	{
		if (binary)
		{
			return true;
		}

		if (in["Scene"])
		{
			m_SceneName = in["Scene"].as<CzuchStr>();
			LOG_BE_INFO("[Scene]Loading scene with name: {0}", m_SceneName);

			if (in["Entities"])
			{
				const YAML::Node& entities = in["Entities"];
				for (auto entity : entities)
				{
					Entity newEntity;
					if (entity["HeaderComponent"])
					{
						auto headerNode = entity["HeaderComponent"];
						CzuchStr tag = headerNode["Tag"].as<CzuchStr>();
						if (tag == "Root")
						{
							newEntity = m_RootEntity;
						}
						else
						{
							newEntity = CreateEntity(headerNode["Name"].as<CzuchStr>(), m_RootEntity);
						}
					}

					if (newEntity.IsValid())
					{
						bool result = newEntity.Deserialize(entity, binary);
						if (result == false)
						{
							LOG_BE_ERROR("[Scene][Load]Failed to load scene. Entity deserialization failed.");
							return false;
						}
					}
					else
					{
						LOG_BE_ERROR("[Scene][Load]Failed to load scene. Entity creation failed.");
						return false;
					}
				}
			}
			else
			{
				LOG_BE_ERROR("[Scene][Load]Failed to load scene. Entities are missing.");
				return false;
			}
		}
		else
		{
			LOG_BE_ERROR("[Scene][Load]Failed to load scene. Scene name is missing.");
			return false;
		}

		return true;
	}
#pragma endregion

}