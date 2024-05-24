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


namespace Czuch
{
	const CzuchStr SceneTag = "SceneControl";

	Scene::Scene(const CzuchStr& sceneName) : m_SceneName(sceneName)
	{
		m_RootEntity = Entity{ m_Registry.create(),this };
		m_RootEntity.AddComponent<HeaderComponent>(sceneName, "Root", Layer{ 0 });
		m_RootEntity.AddComponent<TransformComponent>();

		//create default camera
		Entity cameraEntity = CreateEntity("MainCamera", m_RootEntity);
		auto& cam = cameraEntity.AddComponent<CameraComponent>();
		cam.SetPrimary(true);
		cameraEntity.GetComponent<TransformComponent>().SetLocalPosition({ 0.0f,0.0f,3.0f });
		m_MainCameraEntity = cameraEntity;

		CreateRenderContexts();
	}


	Scene::~Scene()
	{
		m_Registry.clear();
	}

	void Scene::AddUIElement(UIBaseElement* element)
	{
		m_UIElements.push_back(element);
	}

	void Scene::OnUpdate(TimeDelta delta)
	{
		auto view = m_Registry.view<NativeBehaviourComponent, ActiveComponent>();
		for (auto entity : view)
		{
			auto& nativeBehaviour = view.get<NativeBehaviourComponent>(entity);
			nativeBehaviour.OnUpdate(delta);
		}

		for (auto element : m_UIElements)
		{
			element->UpdateUI(delta);
		}
	}

	void Scene::FillRenderContexts(Renderer* renderer)
	{
		//get main camera
		auto cameraView = m_Registry.view<CameraComponent>();
		CameraComponent* mainCamera = nullptr;
		for (auto entity : cameraView)
		{
			auto& camera = cameraView.get<CameraComponent>(entity);
			if (camera.IsPrimary())
			{
				mainCamera = &camera;
				break;
			}
		}

		if (!mainCamera)
		{
			LOG_BE_ERROR("No primary camera in the scene");
			return;
		}

		m_GeneralRenderContext.ClearRenderList();

		Mat4x4 viewMat = mainCamera->GetCamera().GetViewMatrix();

		auto renderableView = m_Registry.view<TransformComponent, MeshComponent, MeshRendererComponent>();
		for (auto entity : renderableView)
		{
			auto& transform = renderableView.get<TransformComponent>(entity);
			auto& mesh = renderableView.get<MeshComponent>(entity);
			auto& meshRenderer = renderableView.get<MeshRendererComponent>(entity);

			for (int a = 0; a < mesh.GetSubMeshesCount(); ++a)
			{
				RenderObjectInstance renderObjectInstance{ .mesh = mesh.GetMesh(a),.overrideMaterial = meshRenderer.GetOverrideMaterial(a), .localToWorldTransformation = Mat4x4(1.0f)/*transform.GetLocalToWorld()*/
				, .localToClipSpaceTransformation = mainCamera->GetCamera().GetViewProjectionMatrix() * transform.GetLocalToWorld() };
				m_GeneralRenderContext.AddToRenderList(renderObjectInstance);
			}
		}
		//find all opaque and transparent entities
		//sort them by layer and sorting order
		//fill render contexts

		//fill ui
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
		if (parent.IsValid())
		{
			parent.AddChild(entity);
		}
		else
		{
			entity.SetAsParent(m_RootEntity);
		}
		return entity;
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
	}

	Entity Scene::GetRootEntity()
	{
		return m_RootEntity;
	}

	void Scene::ClearScene()
	{
		m_Registry.clear();
		m_RootEntity = Entity{ m_Registry.create(),this };
		m_RootEntity.AddComponent<HeaderComponent>(m_SceneName, "Root", Layer{ 0 });
		m_RootEntity.AddComponent<TransformComponent>();
		CreateRenderContexts();
	}

	CameraComponent* Scene::FindPrimaryCamera()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			if (camera.IsPrimary())
			{
				return &camera;
			}
		}
		LOG_BE_ERROR("{0} Scene does not have primary camera",SceneTag);
		return nullptr;
	}

	void Scene::CreateRenderContexts()
	{
		RenderContextCreateInfo renderContextCreateInfo{};
		renderContextCreateInfo.autoClearAfterRender = true;
		renderContextCreateInfo.renderType = RenderType::General;
		renderContextCreateInfo.renderLayer = RenderLayer::LAYER_0;
		renderContextCreateInfo.sortingOrder = 0;
		m_GeneralRenderContext = RenderContext(renderContextCreateInfo);

		renderContextCreateInfo.renderType = RenderType::Debug;
		renderContextCreateInfo.renderLayer = RenderLayer::LAYER_0;
		m_DebugRenderContext = RenderContext(renderContextCreateInfo);
	}

}