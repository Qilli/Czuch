#include "czpch.h"
#include "RenderContext.h"
#include "GraphicsDevice.h"
#include "Subsystems/Scenes/Components/TransformComponent.h"
#include "Subsystems/Scenes/Components/MeshComponent.h"
#include "Subsystems/Scenes/Components/MeshRendererComponent.h"
#include "Subsystems/Scenes/Components/CameraComponent.h"
#include "Subsystems/Scenes/Components/LightComponent.h"
#include "Subsystems/Scenes/Components/NativeBehaviourComponent.h"
#include "Subsystems/Assets/AssetsManager.h"
#include "Subsystems/Scenes/Components/NativeBehaviourComponent.h"
#include"Renderer/RenderPassControl.h"
#include"Renderer/FrameGraph/FrameGraphBuilderHelper.h"

#include"Renderer/Vulkan/RenderPass/VulkanDepthPrepassRenderPass.h"
#include"Renderer/Vulkan/RenderPass/VulkanDefaultForwardLightingRenderPass.h"
#include"Renderer/Vulkan/RenderPass/VulkanDepthLinearPrepassRenderPass.h"
#include"Renderer/Vulkan/RenderPass/VulkanDefaultForwardTransparentLightingRenderPass.h"
#include"Renderer/Vulkan/RenderPass/VulkanDebugDrawRenderPass.h"


namespace Czuch
{
	void RenderObjectInstance::UpdateSceneDataIfRequired(GraphicsDevice* device, SceneDataBuffers& sceneDataBuffers, RenderContextFillParams& fillParams) const
	{
		if (IsValid())
		{
			MaterialInstance* m = nullptr;
			if (fillParams.forceMaterialForAll)
			{
				m = device->AccessMaterialInstance(fillParams.forcedMaterial);
			}
			else
			{
				if (HANDLE_IS_VALID(overrideMaterial))
				{
					m = device->AccessMaterialInstance(overrideMaterial);
				}
				else
				{
					m = device->AccessMaterialInstance(device->AccessMesh(mesh)->materialHandle);
				}
			}

			if (m != nullptr)
			{
				//m->params[passIndex].shaderParamsDesc[0].descriptors[0].resource = sceneDataBuffers.sceneDataBuffer;
				if (m->params[passIndex].setsCount > 0)
				{
					m->params[passIndex].SetUniformBuffer(0, sceneDataBuffers.sceneDataBuffer, 0); //scene data buffer
				}

				if (fillParams.renderPassType == RenderPassType::ForwardLighting || fillParams.renderPassType == RenderPassType::ForwardLightingTransparent)
				{
					//m->params[passIndex].shaderParamsDesc[0].descriptors[1].resource = sceneDataBuffers.renderObjectsBuffer; //render objects buffer
					m->params[passIndex].SetStorageBuffer(0, sceneDataBuffers.renderObjectsBuffer, 1); //render objects buffer
					//m->params[passIndex].shaderParamsDesc[1].descriptors[0].resource = sceneDataBuffers.lightsDataBuffer; //lights data buffer
					m->params[passIndex].SetStorageBuffer(1, sceneDataBuffers.lightsDataBuffer, 0); //lights data buffer
					//m->params[passIndex].shaderParamsDesc[1].descriptors[1].resource = sceneDataBuffers.lightsIndexListBuffer; //lights index buffer
					m->params[passIndex].SetStorageBuffer(1, sceneDataBuffers.lightsIndexListBuffer, 1); //lights index buffer
					//m->params[passIndex].shaderParamsDesc[1].descriptors[2].resource = sceneDataBuffers.tilesDataBuffer; //tiles data buffer	
					m->params[passIndex].SetStorageBuffer(1, sceneDataBuffers.tilesDataBuffer, 2); //tiles data buffer
				}

			}

		}
	}

	void DefaultRenderContext::FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams)
	{
		//ClearRenderList(); // Not needeed probably, we are clearing render lists at the end of frame if necessary

		U32 id = 0;
		U32 counter = 0;

		for (auto& obj : allObjects.allObjects)
		{
			auto currentMaterial = obj.meshRenderer->GetOverrideMaterial();
			auto currentMaterialInstance = device->AccessMaterialInstance(currentMaterial);
			auto material = fillParams.forceMaterialForAll ? (fillParams.ignoreTransparent && currentMaterialInstance->IsTransparent() ? obj.meshRenderer->GetOverrideMaterial() : fillParams.forcedMaterial) : obj.meshRenderer->GetOverrideMaterial();
			auto materialInstance = device->AccessMaterialInstance(material);
			if (!materialInstance)
			{
				LOG_BE_ERROR("Material instance is null");
				continue;
			}

			I32 index = device->AccessMaterial(materialInstance->handle)->GetRenderPassIndexForType(fillParams.renderPassType);
			if (index >= 0)
			{
				RenderObjectInstance renderObjectInstance;

				renderObjectInstance.localToClipSpaceTransformation = cam->GetViewProjectionMatrix() * obj.transform->GetLocalToWorld();
				renderObjectInstance.worldPosition = obj.transform->GetWorldPosition();
				renderObjectInstance.paramData.x =counter;
				renderObjectInstance.mesh = obj.mesh->GetMesh();
				renderObjectInstance.overrideMaterial = material;
				renderObjectInstance.passIndex = index;
				AddToRenderList(renderObjectInstance);
			}
			counter++;
		}
		m_IsDirty = false;
	}

	bool DefaultRenderContext::SupportRenderPass(RenderPassType type) const
	{
		return (U32)type & SUPPORTED_RENDER_PASSES_FLAGS;
	}

	void DefaultTransparentRenderContext::FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams)
	{
		DefaultRenderContext::FillRenderList(device, cam, allObjects, fillParams);

		//sort by distance
		SortRenderObjects(cam);
	}

	bool DefaultTransparentRenderContext::SupportRenderPass(RenderPassType type) const
	{
		return (U32)type & SUPPORTED_RENDER_PASSES_FLAGS;
	}

	void DefaultTransparentRenderContext::SortRenderObjects(Camera* cam)
	{
		Vec3 cameraPosition = cam->GetViewMatrix()[3];
		std::sort(m_RenderObjects.begin(), m_RenderObjects.end(), [cameraPosition](const RenderObjectInstance& a, const RenderObjectInstance& b)
			{
				float distanceA = glm::length(a.worldPosition - cameraPosition);
				float distanceB = glm::length(b.worldPosition - cameraPosition);
				return distanceA > distanceB;
			});
	}

	void DebugRenderContext::FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams)
	{
		//ClearRenderList(); // Not needeed probably, we are clearing render lists at the end of frame if necessary

		MeshHandle meshH;
		meshH.handle = 4998;
		MaterialInstanceHandle materialH;
		materialH = DefaultAssets::DEBUG_DRAW_LIGHT_MATERIAL_INSTANCE;

		for (auto& lightObj : allObjects.allLights)
		{
			RenderObjectInstance renderObjectInstance;
			Mat4x4 localToWorld = lightObj.transform->GetLocalToWorld();
			//leave only position
			localToWorld[0] = Vec4(0.2f, 0, 0, 0);
			localToWorld[1] = Vec4(0, 0.2f, 0, 0);
			localToWorld[2] = Vec4(0, 0, 0.2f, 0);
			localToWorld[3] = Vec4(localToWorld[3].x, localToWorld[3].y, localToWorld[3].z, 1);


			renderObjectInstance.localToClipSpaceTransformation = cam->GetViewProjectionMatrix() * localToWorld;
			renderObjectInstance.worldPosition = localToWorld[3];
			renderObjectInstance.mesh = meshH;
			renderObjectInstance.overrideMaterial = materialH;
			renderObjectInstance.passIndex = 0;
			AddToRenderList(renderObjectInstance);
		}

		m_IsDirty = false;
	}

	bool DebugRenderContext::SupportRenderPass(RenderPassType type) const
	{
		return (U32)type & SUPPORTED_RENDER_PASSES_FLAGS;
	}

	void SceneCameraControl::Release(GraphicsDevice* device)
	{
		frameGraphControl.ReleaseFrameGraph();
		visibleRenderObjects.Clear();
		ReleaseRenderContexts();
		cameraRendering.Release(device);
	}

	void SceneCameraControl::OnSceneActive(Renderer* renderer, GraphicsDevice* device, IScene* scene)
	{
		if (renderer == nullptr)
		{
			return; // this is a call from first scene initialization,scene is not yet active so the renderer is not valid
		}

		if (currentScene == scene)
		{
			LOG_BE_INFO("SceneCameraControl::OnSceneActive: Scene is already active, skipping initialization.");
			return;
		}
		bool handleWindowResize = !EngineRoot::GetEngineSettings().RenderingTargetSizeExternallySet();
		U32 startWidth = handleWindowResize ? device->GetSwapchainWidth() : EngineRoot::GetEngineSettings().targetWidth;
		U32 startHeight = handleWindowResize ? device->GetSwapchainHeight() : EngineRoot::GetEngineSettings().targetHeight;

		frameGraphControl.CreateFrameGraph(camera,device, renderer, Vec2(startWidth, startHeight), handleWindowResize);
		cameraRendering.OnSceneActive(camera, device, scene);
		currentScene = scene;
	}

	void SceneCameraControl::AfterSceneActive()
	{
		if(currentScene ==nullptr)
		{ 
			return; // this is a call from first scene initialization,scene is not yet active so the renderer is not valid
		}
		frameGraphControl.Init();
	}

	void SceneCameraControl::OnResize(GraphicsDevice* device, U32 width, U32 height, bool windowSizeChanged)
	{
		cameraRendering.cameraControl.InitTilesBuffer(device, true, width, height);
		frameGraphControl.OnResize(width, height, windowSizeChanged);
	}

	SceneDataBuffers SceneCameraControl::GetSceneDataBuffers(U32 frame)
	{
		return cameraRendering.cameraControl.GetSceneDataBuffers(frame);
	}

	void SceneCameraControl::UpdateSceneDataBuffers(GraphicsDevice* device, U32 frame, DeletionQueue& deletionQueue)
	{
		cameraRendering.cameraControl.UpdateSceneDataBuffers(cameraRendering.activeScene, device, visibleRenderObjects, frame, deletionQueue);
		cameraRendering.debugCameraControl.UpdateSceneDataBuffer(cameraRendering.activeScene, device, frame);
	}

	RenderContext* SceneCameraControl::GetRenderContext(RenderPassType type, bool createIfNotExist)
	{
		for (auto& ctx : renderContexts)
		{
			if (ctx.SupportsRenderPass(type))
			{
				return ctx.renderContext;
			}
		}

		if (createIfNotExist)
		{
			RenderContextCreateInfo ctx;
			ctx.autoClearBeforeRender = true;
			ctx.renderLayer = RenderLayer::LAYER_0;
			ctx.renderType = RenderType::General;
			ctx.sortingOrder = 0;

			AddRenderContext(ctx, type);
		}
		else
		{
			LOG_BE_ERROR("Render context not found for type: " + std::to_string((int)type));
			return nullptr;
		}
		return GetRenderContext(type);
	}


	void SceneCameraControl::AddRenderContext(RenderContextCreateInfo ctx, RenderPassType type)
	{
		RenderContext* renderContext = nullptr;
		switch (type)
		{
		case RenderPassType::ForwardLighting:
			renderContext = new DefaultRenderContext(ctx);
			break;
		case RenderPassType::DepthPrePass:
		case RenderPassType::DepthLinearPrePass:
			renderContext = new DefaultDepthPrePassRenderContext(ctx);
			break;
		case RenderPassType::ForwardLightingTransparent:
			renderContext = new DefaultTransparentRenderContext(ctx);
			break;
		case RenderPassType::DebugDraw:
			renderContext = new DebugRenderContext(ctx);
			break;
		case RenderPassType::ShadowMap:
			renderContext = new DefaultRenderContext(ctx);
			break;
		default:
			renderContext = new DefaultRenderContext(ctx);
		}
		renderContexts.push_back({ renderContext, type });
	}

	RenderContext* SceneCameraControl::FillRenderList(GraphicsDevice* device, RenderContextFillParams& fillParams)
	{
		auto context = GetRenderContext(fillParams.renderPassType, true);
		if (context->IsDirty())
		{
			context->FillRenderList(device, camera, visibleRenderObjects, fillParams);
		}
		return context;
	}

	void SceneCameraControl::FillDebugDrawElements(GraphicsDevice* device, RenderContextFillParams& fillParams)
	{
		cameraRendering.debugCameraControl.Clear();

		currentScene->ForEachEntity([&](Entity* entity)
			{
				if (entity->HasComponent<NativeBehaviourComponent>())
				{
					auto& behaviour = entity->GetComponent<NativeBehaviourComponent>();
					if (behaviour.IsEnabled())
					{
						behaviour.OnDebugDraw(&cameraRendering.debugCameraControl);
					}
				}

				if (EngineRoot::GetEngineSettings().debugSettings.GetDebugDrawSelectedEntityID() == entity->GetID())
				{
					if (EngineRoot::GetEngineSettings().debugSettings.IsDebugDrawOBBForMeshesEnabled() && entity->HasComponent<MeshRendererComponent>())
					{
						auto& meshRendererComp = entity->GetComponent<MeshRendererComponent>();
						if (meshRendererComp.IsEnabled())
						{
							auto& meshComp = entity->GetComponent<MeshComponent>();
							auto& transformComp = entity->GetComponent<TransformComponent>();
							OBB obb = meshComp.GetOBB(transformComp, EngineRoot::GetEngineSettings().debugSettings.GetDebugDrawOBBForMeshesScale());

							auto& rendering = cameraRendering.debugCameraControl;
							//add lines building obb around mesh
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(-1, -1, -1)), obb.TransformLocalPoint(Vec3(1, -1, -1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(-1, -1, -1)), obb.TransformLocalPoint(Vec3(-1, 1, -1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(-1, -1, -1)), obb.TransformLocalPoint(Vec3(-1, -1, 1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(1, -1, -1)), obb.TransformLocalPoint(Vec3(1, 1, -1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(1, -1, -1)), obb.TransformLocalPoint(Vec3(1, -1, 1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(-1, 1, -1)), obb.TransformLocalPoint(Vec3(1, 1, -1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(-1, 1, -1)), obb.TransformLocalPoint(Vec3(-1, 1, 1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(-1, -1, 1)), obb.TransformLocalPoint(Vec3(1, -1, 1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(-1, -1, 1)), obb.TransformLocalPoint(Vec3(-1, 1, 1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(1, -1, 1)), obb.TransformLocalPoint(Vec3(1, 1, 1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(-1, 1, 1)), obb.TransformLocalPoint(Vec3(1, 1, 1)), Colors::Green);
							rendering.DrawLine(obb.TransformLocalPoint(Vec3(1, 1, 1)), obb.TransformLocalPoint(Vec3(1, 1, -1)), Colors::Green);
						}
					}

					if (EngineRoot::GetEngineSettings().debugSettings.IsDebugDrawNormalForMeshesEnabled() && entity->HasComponent<MeshComponent>() && entity->HasComponent<MeshRendererComponent>())
					{
						auto& meshComp = entity->GetComponent<MeshComponent>();
						auto& transformComp = entity->GetComponent<TransformComponent>();
						auto handle = meshComp.GetMesh();
						if (HANDLE_IS_VALID(handle))
						{
							auto& rendering = cameraRendering.debugCameraControl;
							auto mesh = device->AccessMesh(handle);
							if (mesh != nullptr)
							{
								Mat4x4 localToWorld = transformComp.GetLocalToWorld();
								for (int a = 0; a < mesh->GetMeshData().vertices.size(); ++a)
								{
									const Vec3& posLocal = mesh->GetMeshData().vertices[a].position;
									const Vec3& normalLocal = mesh->GetMeshData().vertices[a].normal;
									Vec3 position = localToWorld * Vec4(posLocal.x, posLocal.y, posLocal.z, 1.0f);
									Vec3 normal = transformComp.GetInverseTransposeLocalToWorld() * Vec4(normalLocal.x, normalLocal.y, normalLocal.z, 0.0f);
									rendering.DrawLine(position, position + normal * 0.2f, Colors::Red);
								}
							}
						}
					}
				}

				if (entity->HasComponent<LightComponent>())
				{
					auto& transform = entity->GetComponent<TransformComponent>();
					auto& lightComp = entity->GetComponent<LightComponent>();
					if (lightComp.IsEnabled())
					{
						auto& rendering = cameraRendering.debugCameraControl;
						switch (lightComp.GetLightType())
						{
						case LightType::Point:
							rendering.DrawLinesSphere(transform.GetWorldPosition(), lightComp.GetLightRange(), Colors::Yellow);
							break;
						case LightType::Spot:
							rendering.DrawCone(transform.GetWorldPosition(), transform.GetWorldForward(), lightComp.GetLightRange(), lightComp.GetOuterAngle(), Colors::Yellow);
							break;
						case LightType::Directional:
							rendering.DrawCircle(transform.GetWorldPosition(), transform.GetWorldForward(), 0.2f, Colors::Yellow);
							rendering.DrawLine(transform.GetWorldPosition(), transform.GetWorldPosition() + transform.GetWorldForward() * 0.5f, Colors::Yellow);

							Mat3x3 forwardSpace = GetNewSpaceOrientation(transform.GetWorldPosition(), transform.GetWorldForward());

							//draw a few line in a circle along right and up direction of this new local space
							for (int i = 0; i < 360; i += 30)
							{
								float angle = glm::radians(static_cast<float>(i));
								Vec3 localPosition = Vec3(cos(angle), 0, sin(angle)) * 0.2f;
								Vec3 targetPos = forwardSpace[0] * localPosition.x + forwardSpace[1] * localPosition.z;
								rendering.DrawLine(transform.GetWorldPosition() + targetPos, transform.GetWorldPosition() + targetPos + transform.GetWorldForward() * 0.5f, Colors::Yellow);
							}

							break;
						default:
							break;
						}
					}
				}
			});
	}

	IndirectDrawForCommandBufferData& SceneCameraControl::GetIndirectDrawDataForDebugDrawingLines(RenderContextFillParams& fillParams, U32 frame)
	{
		return cameraRendering.debugCameraControl.FillAndGetIndirectDrawDataForDebugLinesDrawing(fillParams.forcedMaterial, frame);
	}

	IndirectDrawForCommandBufferData& SceneCameraControl::GetIndirectDrawDataForDebugDrawingTriangles(RenderContextFillParams& fillParams, U32 frame)
	{
		return cameraRendering.debugCameraControl.FillAndGetIndirectDrawDataForDebugTrianglesDrawing(fillParams.forcedMaterial, frame);
	}

	IndirectDrawForCommandBufferData& SceneCameraControl::GetIndirectDrawDataForDebugDrawingPoints(RenderContextFillParams& fillParams, U32 frame)
	{
		return cameraRendering.debugCameraControl.FillAndGetIndirectDrawDataForDebugPointsDrawing(fillParams.forcedMaterial, frame);
	}

	void SceneCameraControl::OnPostRender()
	{
		for (auto& ctx : renderContexts)
		{
			if (ctx.renderContext->IsAutoCleanEnabled())
			{
				ctx.renderContext->ClearRenderList();
			}
		}
	}

	void SceneCameraControl::UpdateVisibleObjects(RenderObjectsContainer& allObjects)
	{
		visibleRenderObjects.Clear();

		for (auto& obj : allObjects.allObjects)
		{
			visibleRenderObjects.allObjects.push_back(obj);
		}

		for (auto& obj : allObjects.allLights)
		{
			visibleRenderObjects.allLights.push_back(obj);
		}

	}

	void SceneCameraControl::ReleaseRenderContexts()
	{
		for (auto& ctx : renderContexts)
		{
			if (ctx.renderContext != nullptr)
			{
				delete ctx.renderContext;
				ctx.renderContext = nullptr;
			}
		}
		renderContexts.clear();
	}

	void SceneCameraRendering::Release(GraphicsDevice* device)
	{
		cameraControl.Release(device);
		debugCameraControl.Release(device);
	}

	void SceneCameraRendering::OnSceneActive(Camera* camera, GraphicsDevice* device, IScene* scene)
	{
		activeScene = scene;
		cameraControl.Init(device, camera);
		cameraControl.OnSceneActive(device);
		debugCameraControl.OnSceneActive(camera, device);
	}

	void SceneCameraRenderingControl::Init(GraphicsDevice* device, Camera* cam)
	{
		lightsIndexList.reserve(MAX_LIGHTS_IN_SCENE * MAX_LIGHTS_IN_SCENE);
		tilesDataContainer.tilesData.reserve(MAX_LIGHTS_IN_SCENE * MAX_LIGHTS_IN_SCENE);
		lightsData.reserve(MAX_LIGHTS_IN_SCENE);
		renderObjectsData.reserve(INIT_MAX_RENDER_OBJECTS);
		camera = cam;
	}

	void SceneCameraRenderingControl::OnSceneActive(GraphicsDevice* device)
	{
		sceneDataBufferDesc.persistentMapped = true;
		sceneDataBufferDesc.elementsCount = 1;
		sceneDataBufferDesc.bind_flags = BindFlag::UNIFORM_BUFFER;
		sceneDataBufferDesc.size = sizeof(SceneData);
		sceneDataBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		lightsBufferDesc.persistentMapped = true;
		lightsBufferDesc.elementsCount = MAX_LIGHTS_IN_SCENE;
		lightsBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		lightsBufferDesc.size = sizeof(LightData) * MAX_LIGHTS_IN_SCENE;
		lightsBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		InitTilesBuffer(device, false, 0, 0);
		InitRenderObjectsBuffer(device, false, INIT_MAX_RENDER_OBJECTS);

		sceneData.ambientColor = Vec4(0.0f, 0.0f,0.0f, 1);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			device->Release(sceneDataBuffer[a]);
			device->Release(lightsBuffer[a]);
			lightsBuffer[a] = device->CreateBuffer(&lightsBufferDesc);
			sceneDataBuffer[a] = device->CreateBuffer(&sceneDataBufferDesc);
		}
	}

	void SceneCameraRenderingControl::Release(GraphicsDevice* device)
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			device->Release(tilesBuffer[i]);
			device->Release(lightsListBuffer[i]);
			device->Release(lightsBuffer[i]);
			device->Release(renderObjectsBuffer[i]);
		}
	}

	void SceneCameraRenderingControl::InitTilesBuffer(GraphicsDevice* device, bool resize, U32 width, U32 height)
	{
		if (width == 0 || height == 0)
		{
			width = device->GetSwapchainWidth();
			height = device->GetSwapchainHeight();
		}

		tiles_in_width = static_cast<U32>(std::ceil(width / (float)TILE_SIZE));
		tiles_in_height = static_cast<U32>(std::ceil(height / (float)TILE_SIZE));


		tiles_count = tiles_in_width * tiles_in_height;

		tilesBufferDesc.persistentMapped = true;
		tilesBufferDesc.elementsCount = 1;
		tilesBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		tilesBufferDesc.size = sizeof(LightsTileData) * tiles_count;
		tilesBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		lightsListBufferDesc.persistentMapped = true;
		lightsListBufferDesc.elementsCount = 1;
		lightsListBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		lightsListBufferDesc.size = sizeof(U32) * MAX_LIGHTS_IN_TILE * tiles_count + sizeof(glm::ivec4);
		lightsListBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			device->Release(tilesBuffer[i]);
			device->Release(lightsListBuffer[i]);
		}

		LOG_BE_INFO("Tiles count: " + std::to_string(tiles_count) + " tiles in width: " + std::to_string(tiles_in_width) + " tiles in height: " + std::to_string(tiles_in_height));

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			tilesBuffer[i] = device->CreateBuffer(&tilesBufferDesc);
			lightsListBuffer[i] = device->CreateBuffer(&lightsListBufferDesc);
		}

		tilesDataContainer.screenSize = glm::ivec4(width, height, tiles_in_width, tiles_in_height);

		//update materials for elements size?
		UpdateMaterialsLightsInfo();
	}

	SceneDataBuffers SceneCameraRenderingControl::GetSceneDataBuffers(U32 frame)
	{
		return {
			sceneDataBuffer[frame],
			lightsBuffer[frame],
			lightsListBuffer[frame],
			tilesBuffer[frame],
			renderObjectsBuffer[frame]};
	}

	void SceneCameraRenderingControl::UpdateSceneDataBuffers(IScene* scene, GraphicsDevice* device, RenderObjectsContainer& visibleObjects, U32 frame, DeletionQueue& deletionQueue)
	{
		if (scene != nullptr)
		{
			auto& lights = scene->GetAllLightObjects();
			if (FillTilesWithLights(device, lights, frame))
			{
				UpdateMaterialsLightsInfo();
			}

			if (FillRenderObjectsData(device, visibleObjects, frame))
			{
				//Update materials render elements info
				UpdateMaterialsRenderObjectsInfo();
			}

			sceneData.view = camera->GetViewMatrix();
			sceneData.proj = camera->GetProjectionMatrix();
			sceneData.viewproj = camera->GetViewProjectionMatrix();
			sceneData.cameraWorldPosition = camera->GetWorldPosition();
			sceneData.ambientColor = scene->GetAmbientColor();
		}

		auto bufferVulkan = device->GetMappedBufferDataPtr(sceneDataBuffer[frame]);
		uint8_t* byteData = static_cast<uint8_t*>(bufferVulkan);
		memcpy(byteData, &sceneData, sizeof(SceneData));
	}


	void SceneCameraRenderingControl::InitRenderObjectsBuffer(GraphicsDevice* device, bool resize, U32 size)
	{
		if (resize)
		{
			for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
			{
				device->Release(renderObjectsBuffer[frame]);
				INVALIDATE_HANDLE(renderObjectsBuffer[frame]);
			}
		}

		renderObjectsBufferDesc.persistentMapped = true;
		renderObjectsBufferDesc.elementsCount = 1;
		renderObjectsBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		renderObjectsBufferDesc.size = sizeof(RenderObjectGPUData) * size;
		renderObjectsBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
		{
			renderObjectsBuffer[frame] = device->CreateBuffer(&renderObjectsBufferDesc);
		}

		renderObjectsData.reserve(size);

		if (resize == false)
		{
			UpdateMaterialsRenderObjectsInfo();
		}
	}

	void SceneCameraRenderingControl::UpdateMaterialsLightsInfo()
	{
		AssetsManager::Get().UpdateLightingMaterialsLightInfo(MAX_LIGHTS_IN_SCENE, MAX_LIGHTS_IN_TILE * tiles_count, tiles_count, MATERIAL_DATA_CAPACITY);
	}

	void SceneCameraRenderingControl::UpdateMaterialsRenderObjectsInfo()
	{
		AssetsManager::Get().UpdateRenderObjectsInfo(renderObjectsData.capacity());
	}

	bool SceneCameraRenderingControl::FillTilesWithLights(GraphicsDevice* device, const Array<LightObjectInfo>& allLights, U32 frame)
	{
		lightsIndexList.clear();
		tilesDataContainer.tilesData.clear();
		lightsData.clear();

		//fill lights data
		auto bufferVulkanLights = device->GetMappedBufferDataPtr(lightsBuffer[frame]);
		LightData* lightsData = (LightData*)bufferVulkanLights;
		for (int a = 0; a < allLights.size(); ++a)
		{
			lightsData[a] = allLights[a].lightData;
		}

		//we want to check if number of lights changed so we know if we need to update material info about lights
		if (lastLightsCount != allLights.size())
		{
			lightsChanged = true;
			lastLightsCount = allLights.size();
		}

		for (int i = 0; i < tiles_count; ++i)
		{
			//here compute visible lights for each tile
			//and store them in tilesData

			//for now we will just store all lights in each tile
			int startIndex = lightsIndexList.size();
			U32 count = allLights.size() <= MAX_LIGHTS_IN_TILE ? allLights.size() : MAX_LIGHTS_IN_TILE;
			for (int a = 0; a < count; ++a)
			{
				lightsIndexList.push_back(a);
			}
			int endIndex = lightsIndexList.size() - 1;

			LightsTileData tileData;
			tileData.lightStart = startIndex;
			tileData.lightCount = endIndex - startIndex + 1;
			tilesDataContainer.tilesData.push_back(std::move(tileData));
		}

		//fill tiled data buffer with data
		auto bufferVulkan = device->GetMappedBufferDataPtr(tilesBuffer[frame]);
		void* data = bufferVulkan;
		uint8_t* byteData = static_cast<uint8_t*>(data);
		// Copy tile data after screenSize
		memcpy(byteData, tilesDataContainer.tilesData.data(), sizeof(LightsTileData) * tilesDataContainer.tilesData.size());

		//fill lights list buffer
		void* lightsBufferData = device->GetMappedBufferDataPtr(lightsListBuffer[frame]);
		uint8_t* lightsListData = (uint8_t*)lightsBufferData;
		glm::ivec4 screenSize = tilesDataContainer.screenSize;
		//copy screen size to the beginning of the buffer
		memcpy(lightsListData, &screenSize, sizeof(glm::ivec4));
		//do memcopy
		memcpy(lightsListData + sizeof(glm::ivec4), lightsIndexList.data(), sizeof(U32) * lightsIndexList.size() + sizeof(glm::ivec4));

		bool localLightsChanged = lightsChanged;
		lightsChanged = false;
		return localLightsChanged;
	}

	bool SceneCameraRenderingControl::FillRenderObjectsData(GraphicsDevice* device, RenderObjectsContainer& allObjects, U32 frame)
	{
		bool changed = false;
		if (renderObjectsData.capacity() < allObjects.allObjects.size())
		{
			InitRenderObjectsBuffer(device, true, allObjects.allObjects.size() * 2);
			changed = true;
		}

		renderObjectsData.clear();

		for (auto& obj : allObjects.allObjects)
		{
			MaterialInstance* materialInstance = device->AccessMaterialInstance(obj.meshRenderer->GetOverrideMaterial());
			RenderObjectGPUData renderObjectData;
			renderObjectData.localToWorldTransformation = obj.transform->GetLocalToWorld();
			renderObjectData.invTransposeToWorldMatrix = glm::inverse(glm::transpose((renderObjectData.localToWorldTransformation)));
			renderObjectData.materialAndFlags.x = materialInstance->GetIndexForInternalBufferForTag(DescriptorBindingTagType::MATERIALS_LIGHTING_DATA,0);
			renderObjectsData.push_back(std::move(renderObjectData));
		}

		auto bufferVulkan = device->GetMappedBufferDataPtr(renderObjectsBuffer[frame]);
		uint8_t* byteData = static_cast<uint8_t*>(bufferVulkan);
		memcpy(byteData, renderObjectsData.data(), sizeof(RenderObjectGPUData) * renderObjectsData.size());
		return true;
	}

	bool RenderContext::SupportRenderPass(RenderPassType type) const
	{
		return false;
	}

	bool RenderContextControl::SupportsRenderPass(RenderPassType type) const
	{
		if (renderPassType == type)
		{
			return true;
		}
		return renderContext->SupportRenderPass(type);
	}

	void DefaultDepthPrePassRenderContext::FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams)
	{
		//ClearRenderList(); // Not needeed probably, we are clearing render lists at the end of frame if necessary

		U32 id = 0;

		for (auto& obj : allObjects.allObjects)
		{
			auto currentMaterial = obj.meshRenderer->GetOverrideMaterial();
			auto currentMaterialInstance = device->AccessMaterialInstance(currentMaterial);

			if (fillParams.ignoreTransparent && currentMaterialInstance->IsTransparent())
			{
				continue;
			}

			auto material = fillParams.forcedMaterial;
			auto materialInstance = device->AccessMaterialInstance(material);
			if (!materialInstance)
			{
				LOG_BE_ERROR("Material instance is null");
				continue;
			}

			I32 index = device->AccessMaterial(materialInstance->handle)->GetRenderPassIndexForType(fillParams.renderPassType);
			if (index >= 0)
			{
				RenderObjectInstance renderObjectInstance;

				renderObjectInstance.localToClipSpaceTransformation = cam->GetViewProjectionMatrix() * obj.transform->GetLocalToWorld();
				renderObjectInstance.worldPosition = obj.transform->GetWorldPosition();
				renderObjectInstance.paramData.x = 0;
				renderObjectInstance.mesh = obj.mesh->GetMesh();
				renderObjectInstance.overrideMaterial = material;
				renderObjectInstance.passIndex = index;
				AddToRenderList(renderObjectInstance);
			}
		}
		m_IsDirty = false;
	}

	bool DefaultDepthPrePassRenderContext::SupportRenderPass(RenderPassType type) const
	{
		return (U32)type & SUPPORTED_RENDER_PASSES_FLAGS;
	}

	PositionVertex LineBasicVertices[] = {
		{ Vec3(0, 0, 0)},
		{ Vec3(1, 0, 0)},
	};

	U32 LineBasicIndices[] = {
		0, 1
	};

	PositionVertex TriangleBasicVertices[] = {
		{ Vec3(0, 0, 0)},
		{ Vec3(1, 0, 0)},
		{ Vec3(0, 1, 0)},
	};

	U32 TriangleBasicIndices[] = {
		0, 1, 2
	};

	PositionVertex PointBasicVertices[] = {
		{ Vec3(0, 0, 0)},
	};

	U32 PointBasicIndices[] = {
		0
	};


	void SceneCameraDebugRenderingControl::CreateAndInitLinesBuffer(GraphicsDevice* device)
	{
		m_LinesBufferDesc.persistentMapped = true;
		m_LinesBufferDesc.elementsCount = 1;
		m_LinesBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		m_LinesBufferDesc.size = sizeof(LineInstanceData) * MAX_LINES_IN_SCENE;
		m_LinesBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		m_VertexBufferLinesDesc.persistentMapped = false;
		m_VertexBufferLinesDesc.elementsCount = 1;
		m_VertexBufferLinesDesc.bind_flags = BindFlag::VERTEX_BUFFER;
		m_VertexBufferLinesDesc.size = sizeof(PositionVertex) * 2;
		m_VertexBufferLinesDesc.usage = Usage::DEFAULT;
		m_VertexBufferLinesDesc.stride = sizeof(PositionVertex);
		m_VertexBufferLinesDesc.initData = LineBasicVertices;

		m_IndexBufferLinesDesc.persistentMapped = false;
		m_IndexBufferLinesDesc.elementsCount = 1;
		m_IndexBufferLinesDesc.bind_flags = BindFlag::INDEX_BUFFER;
		m_IndexBufferLinesDesc.size = sizeof(U32) * 2;
		m_IndexBufferLinesDesc.usage = Usage::DEFAULT;
		m_IndexBufferLinesDesc.stride = sizeof(U32);
		m_IndexBufferLinesDesc.initData = LineBasicIndices;

		m_VertexBufferLines = device->CreateBuffer(&m_VertexBufferLinesDesc);
		m_IndexBufferLines = device->CreateBuffer(&m_IndexBufferLinesDesc);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_LinesBuffer[a] = device->CreateBuffer(&m_LinesBufferDesc);
		}
	}

	void SceneCameraDebugRenderingControl::ReleaseLinesBuffer(GraphicsDevice* device)
	{
		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			device->Release(m_LinesBuffer[a]);
			INVALIDATE_HANDLE(m_LinesBuffer[a]);
		}
		device->Release(m_VertexBufferLines);
		device->Release(m_IndexBufferLines);
		m_Lines.clear();
	}

	void SceneCameraDebugRenderingControl::CreateAndInitTrianglesBuffer(GraphicsDevice* device)
	{
		m_TrianglesBufferDesc.persistentMapped = true;
		m_TrianglesBufferDesc.elementsCount = 1;
		m_TrianglesBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		m_TrianglesBufferDesc.size = sizeof(TriangleInstanceData) * MAX_DEBUG_TRIANGLES_IN_SCENE;
		m_TrianglesBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		m_VertexBufferTrianglesDesc.persistentMapped = false;
		m_VertexBufferTrianglesDesc.elementsCount = 1;
		m_VertexBufferTrianglesDesc.bind_flags = BindFlag::VERTEX_BUFFER;
		m_VertexBufferTrianglesDesc.size = sizeof(PositionVertex) * 3;
		m_VertexBufferTrianglesDesc.usage = Usage::DEFAULT;
		m_VertexBufferTrianglesDesc.stride = sizeof(PositionVertex);
		m_VertexBufferTrianglesDesc.initData = TriangleBasicVertices;

		m_IndexBufferTrianglesDesc.persistentMapped = false;
		m_IndexBufferTrianglesDesc.elementsCount = 1;
		m_IndexBufferTrianglesDesc.bind_flags = BindFlag::INDEX_BUFFER;
		m_IndexBufferTrianglesDesc.size = sizeof(U32) * 3;
		m_IndexBufferTrianglesDesc.usage = Usage::DEFAULT;
		m_IndexBufferTrianglesDesc.stride = sizeof(U32);
		m_IndexBufferTrianglesDesc.initData = TriangleBasicIndices;

		m_VertexBufferTriangles = device->CreateBuffer(&m_VertexBufferTrianglesDesc);
		m_IndexBufferTriangles = device->CreateBuffer(&m_IndexBufferTrianglesDesc);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_TrianglesBuffer[a] = device->CreateBuffer(&m_TrianglesBufferDesc);
		}
	}

	void SceneCameraDebugRenderingControl::ReleaseTrianglesBuffer(GraphicsDevice* device)
	{
		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			device->Release(m_TrianglesBuffer[a]);
			INVALIDATE_HANDLE(m_TrianglesBuffer[a]);
		}
		device->Release(m_VertexBufferTriangles);
		device->Release(m_IndexBufferTriangles);
		m_Triangles.clear();
	}

	void SceneCameraDebugRenderingControl::CreateAndInitPointsBuffer(GraphicsDevice* device)
	{
		m_PointsBufferDesc.persistentMapped = true;
		m_PointsBufferDesc.elementsCount = 1;
		m_PointsBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		m_PointsBufferDesc.size = sizeof(PointInstanceData) * MAX_DEBUG_POINTS_IN_SCENE;
		m_PointsBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		m_VertexBufferPointsDesc.persistentMapped = false;
		m_VertexBufferPointsDesc.elementsCount = 1;
		m_VertexBufferPointsDesc.bind_flags = BindFlag::VERTEX_BUFFER;
		m_VertexBufferPointsDesc.size = sizeof(PositionVertex);
		m_VertexBufferPointsDesc.usage = Usage::DEFAULT;
		m_VertexBufferPointsDesc.stride = sizeof(PositionVertex);
		m_VertexBufferPointsDesc.initData = PointBasicVertices;

		m_VertexBufferPoints = device->CreateBuffer(&m_VertexBufferPointsDesc);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_PointsBuffer[a] = device->CreateBuffer(&m_PointsBufferDesc);
		}
	}

	void SceneCameraDebugRenderingControl::ReleasePointsBuffer(GraphicsDevice* device)
	{
		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			device->Release(m_PointsBuffer[a]);
			INVALIDATE_HANDLE(m_PointsBuffer[a]);
		}
		device->Release(m_VertexBufferPoints);
		m_Points.clear();
	}

	void SceneCameraDebugRenderingControl::UpdateSceneDataBuffer(IScene* scene, GraphicsDevice* device, U32 frame)
	{
		if (scene != nullptr)
		{
			m_SceneData.view = m_Camera->GetViewMatrix();
			m_SceneData.proj = m_Camera->GetProjectionMatrix();
			m_SceneData.viewproj = m_Camera->GetViewProjectionMatrix();
			m_SceneData.cameraWorldPosition = m_Camera->GetWorldPosition();
			m_SceneData.ambientColor = scene->GetAmbientColor();

			if (FillDebugBuffersData(device, frame))
			{
				UpdateDebugMaterialInfo();
			}
		}

		auto bufferVulkan = device->GetMappedBufferDataPtr(m_SceneBuffer[frame]);
		uint8_t* byteData = static_cast<uint8_t*>(bufferVulkan);
		memcpy(byteData, &m_SceneData, sizeof(SceneData));
	}

	void SceneCameraDebugRenderingControl::OnSceneActive(Camera* cam, GraphicsDevice* device)
	{
		m_Camera = cam;

		m_SceneBufferDesc.persistentMapped = true;
		m_SceneBufferDesc.elementsCount = 1;
		m_SceneBufferDesc.bind_flags = BindFlag::UNIFORM_BUFFER;
		m_SceneBufferDesc.size = sizeof(SceneData);
		m_SceneBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		m_CommandsBufferDesc.persistentMapped = true;
		m_CommandsBufferDesc.elementsCount = 1;
		m_CommandsBufferDesc.bind_flags = BindFlag::INDIRECT_BUFFER;
		m_CommandsBufferDesc.size = sizeof(DrawIndexedIndirectCommand) * 3;
		m_CommandsBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		Release(device);

		CreateAndInitLinesBuffer(device);
		CreateAndInitTrianglesBuffer(device);
		CreateAndInitPointsBuffer(device);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			m_SceneBuffer[a] = device->CreateBuffer(&m_SceneBufferDesc);
			m_CommandsBuffer[a] = device->CreateBuffer(&m_CommandsBufferDesc);
		}

	}

	void SceneCameraDebugRenderingControl::Release(GraphicsDevice* device)
	{
		ReleaseLinesBuffer(device);
		ReleaseTrianglesBuffer(device);
		ReleasePointsBuffer(device);
		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			device->Release(m_SceneBuffer[a]);
			device->Release(m_CommandsBuffer[a]);
			INVALIDATE_HANDLE(m_SceneBuffer[a]);
			INVALIDATE_HANDLE(m_CommandsBuffer[a]);
		}
		m_SceneData = {};
	}

	bool SceneCameraDebugRenderingControl::FillDebugBuffersData(GraphicsDevice* device, U32 frame)
	{
		if (m_Lines.size() < MAX_LINES_IN_SCENE)
		{

			auto bufferVulkan = device->GetMappedBufferDataPtr(m_LinesBuffer[frame]);
			uint8_t* byteData = static_cast<uint8_t*>(bufferVulkan);
			memcpy(byteData, m_Lines.data(), sizeof(LineInstanceData) * m_Lines.size());

			auto bufferVulkanCommands = device->GetMappedBufferDataPtr(m_CommandsBuffer[frame]);
			DrawIndexedIndirectCommand* commands = (DrawIndexedIndirectCommand*)bufferVulkanCommands;

			//Lines instances info
			commands[0].indexCount = 2;
			commands[0].instanceCount = static_cast<U32>(m_Lines.size());
			commands[0].firstIndex = 0;
			commands[0].vertexOffset = 0;
			commands[0].firstInstance = 0;
		}

		if (m_Triangles.size() < MAX_DEBUG_TRIANGLES_IN_SCENE)
		{
			auto bufferVulkanTriangles = device->GetMappedBufferDataPtr(m_TrianglesBuffer[frame]);
			uint8_t* byteDataTriangles = static_cast<uint8_t*>(bufferVulkanTriangles);
			memcpy(byteDataTriangles, m_Triangles.data(), sizeof(TriangleInstanceData) * m_Triangles.size());

			auto bufferVulkanCommands = device->GetMappedBufferDataPtr(m_CommandsBuffer[frame]);
			DrawIndexedIndirectCommand* commands = (DrawIndexedIndirectCommand*)bufferVulkanCommands;

			//Triangles instances info
			commands[1].indexCount = 3;
			commands[1].instanceCount = static_cast<U32>(m_Triangles.size());
			commands[1].firstIndex = 0;
			commands[1].vertexOffset = 0;
			commands[1].firstInstance = 0;
		}

		if (m_Points.size() < MAX_DEBUG_POINTS_IN_SCENE)
		{
			auto bufferVulkanPoints = device->GetMappedBufferDataPtr(m_PointsBuffer[frame]);
			uint8_t* byteDataPoints = static_cast<uint8_t*>(bufferVulkanPoints);
			memcpy(byteDataPoints, m_Points.data(), sizeof(PointInstanceData) * m_Points.size());

			auto bufferVulkanCommands = device->GetMappedBufferDataPtr(m_CommandsBuffer[frame]);
			DrawIndexedIndirectCommand* commands = (DrawIndexedIndirectCommand*)bufferVulkanCommands;

			//Points instances info
			commands[2].indexCount = 1;
			commands[2].instanceCount = static_cast<U32>(m_Points.size());
			commands[2].firstIndex = 0;
			commands[2].vertexOffset = 0;
			commands[2].firstInstance = 0;
		}

		return true;
	}

	void SceneCameraDebugRenderingControl::UpdateDebugMaterialInfo()
	{
	}

	IndirectDrawForCommandBufferData& SceneCameraDebugRenderingControl::FillAndGetIndirectDrawDataForDebugLinesDrawing(MaterialInstanceHandle material, U32 frame)
	{
		m_IndirectDrawDataLines.binding = 0;
		m_IndirectDrawDataLines.indexBuffer = m_IndexBufferLines;
		m_IndirectDrawDataLines.vertexBuffer = m_VertexBufferLines;
		m_IndirectDrawDataLines.instancesBuffer = m_LinesBuffer[frame];
		m_IndirectDrawDataLines.sceneDataBuffer = m_SceneBuffer[frame];
		m_IndirectDrawDataLines.material = material;
		m_IndirectDrawDataLines.indirectDrawsCommandsBuffer = m_CommandsBuffer[frame];
		m_IndirectDrawDataLines.indirectDrawsCommandsOffset = 0;
		m_IndirectDrawDataLines.instancesSize = sizeof(LineInstanceData) * m_Lines.size();
		m_IndirectDrawDataLines.set = 1;
		m_IndirectDrawDataLines.binding = 0;

		return m_IndirectDrawDataLines;
	}

	IndirectDrawForCommandBufferData& SceneCameraDebugRenderingControl::FillAndGetIndirectDrawDataForDebugTrianglesDrawing(MaterialInstanceHandle material, U32 frame)
	{
		m_IndirectDrawDataTriangles.binding = 0;
		m_IndirectDrawDataTriangles.indexBuffer = m_IndexBufferTriangles;
		m_IndirectDrawDataTriangles.vertexBuffer = m_VertexBufferTriangles;
		m_IndirectDrawDataTriangles.instancesBuffer = m_TrianglesBuffer[frame];
		m_IndirectDrawDataTriangles.sceneDataBuffer = m_SceneBuffer[frame];
		m_IndirectDrawDataTriangles.material = material;
		m_IndirectDrawDataTriangles.indirectDrawsCommandsBuffer = m_CommandsBuffer[frame];
		m_IndirectDrawDataTriangles.indirectDrawsCommandsOffset = sizeof(DrawIndexedIndirectCommand);
		m_IndirectDrawDataTriangles.instancesSize = sizeof(TriangleInstanceData) * m_Triangles.size();
		m_IndirectDrawDataTriangles.set = 1;
		m_IndirectDrawDataTriangles.binding = 0;

		return m_IndirectDrawDataTriangles;
	}

	IndirectDrawForCommandBufferData& SceneCameraDebugRenderingControl::FillAndGetIndirectDrawDataForDebugPointsDrawing(MaterialInstanceHandle material, U32 frame)
	{
		m_IndirectDrawDataPoints.binding = 0;
		m_IndirectDrawDataPoints.indexBuffer = BufferHandle();
		m_IndirectDrawDataPoints.vertexBuffer = m_VertexBufferPoints;
		m_IndirectDrawDataPoints.instancesBuffer = m_PointsBuffer[frame];
		m_IndirectDrawDataPoints.sceneDataBuffer = m_SceneBuffer[frame];
		m_IndirectDrawDataPoints.material = material;
		m_IndirectDrawDataPoints.indirectDrawsCommandsBuffer = m_CommandsBuffer[frame];
		m_IndirectDrawDataPoints.indirectDrawsCommandsOffset = sizeof(DrawIndexedIndirectCommand) * 2;
		m_IndirectDrawDataPoints.instancesSize = sizeof(PointInstanceData) * m_Points.size();
		m_IndirectDrawDataPoints.set = 1;
		m_IndirectDrawDataPoints.binding = 0;

		return m_IndirectDrawDataPoints;
	}

	void SceneCameraDebugRenderingControl::DrawLine(const Vec3& start, const Vec3& end, const Color& color)
	{
		if (m_Lines.size() >= MAX_LINES_IN_SCENE)
		{
			return;
		}
		m_Lines.push_back(LineInstanceData(start, end, color));
	}

	void SceneCameraDebugRenderingControl::DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color)
	{
		if (m_Triangles.size() < MAX_DEBUG_TRIANGLES_IN_SCENE)
		{
			m_Triangles.push_back(TriangleInstanceData(a, b, c, color));
		}
		else
		{
			LOG_BE_ERROR("Maximum number of debug triangles reached: " + std::to_string(MAX_DEBUG_TRIANGLES_IN_SCENE));
		}
	}

	void SceneCameraDebugRenderingControl::DrawPoint(const Vec3& point, const Color& color, const float size)
	{
		if (m_Points.size() < MAX_DEBUG_POINTS_IN_SCENE)
		{
			m_Points.push_back(PointInstanceData(point, color, size));
		}
		else
		{
			LOG_BE_ERROR("Maximum number of debug points reached: " + std::to_string(MAX_DEBUG_POINTS_IN_SCENE));
		}
	}

	void SceneCameraDebugRenderingControl::DrawMesh(const AssetHandle mesh, const Mat4x4& transform, const Color& color)
	{
	}

	void SceneCameraDebugRenderingControl::DrawQuad(const Vec3& center, const Vec3& normal, float size, const Color& color)
	{
		Vec3 normalIn = glm::normalize(normal);
		Vec3 right;
		if (glm::abs(glm::dot(normal, Vec3(0, 1.0f, 0))) > 0.99f)
		{
			right = Vec3(1, 0, 0);
		}
		else
		{
			right = glm::normalize(glm::cross(normalIn, Vec3(0, 1, 0)));
		}
		Vec3 up = glm::normalize(glm::cross(right, normalIn)) * size;

		Vec3 a = center - right - up;
		Vec3 b = center + right - up;
		Vec3 c = center + right + up;
		Vec3 d = center - right + up;

		DrawTriangle(a, b, c, color);
		DrawTriangle(a, c, d, color);
	}

	void SceneCameraDebugRenderingControl::DrawCircle(const Vec3& center, const Vec3& normal, float radius, const Color& color)
	{
		//compute axes in the plane of the circle
		Vec3 normalIn = glm::normalize(normal);
		Vec3 right;
		if (glm::abs(glm::dot(normalIn, Vec3(0, 1.0f, 0))) > 0.99f)
		{
			right = Vec3(1, 0, 0);
		}
		else
		{
			right = glm::normalize(glm::cross(normalIn, Vec3(0, 1, 0)));
		}
		Vec3 up = glm::normalize(glm::cross(right, normalIn));
		float numSegments = 32; // Number of segments for the circle
		Vec3 worldPosition = center;

		for (int i = 0; i < numSegments; ++i) {
			float angle = (2.0f * glm::pi<float>() * static_cast<float>(i)) / static_cast<float>(numSegments);
			float x = radius * glm::cos(angle);
			float y = radius * glm::sin(angle);

			// Calculate the point on the circle in 3D space
			Vec3 point1 = worldPosition + x * right + y * up;

			float nextAngle = (2.0f * glm::pi<float>() * static_cast<float>(i + 1)) / static_cast<float>(numSegments);
			float nextX = radius * glm::cos(nextAngle);
			float nextY = radius * glm::sin(nextAngle);

			// Calculate the next point on the circle in 3D space
			Vec3 point2 = worldPosition + nextX * right + nextY * up;

			// Draw the line between the two points
			DrawLine(point1, point2, color);
		}
	}

	void SceneCameraDebugRenderingControl::DrawLinesSphere(const Vec3& center, float radius, const Color& color)
	{
		DrawCircle(center, Vec3(0, 1, 0), radius, color);
		DrawCircle(center, Vec3(1, 0, 0), radius, color);
		DrawCircle(center, Vec3(0, 0, 1), radius, color);
	}

	void SceneCameraDebugRenderingControl::DrawCone(const Vec3& position, const Vec3& direction, float range, float angle, const Color& color)
	{
		Vec3 forward = glm::normalize(direction);
		Vec3 right = glm::normalize(glm::cross(forward, Vec3(0, 1, 0)));
		Vec3 up = glm::normalize(glm::cross(right, forward));

		Vec3 baseCenter = position + forward * range;
		float radius = range * tan(angle);

		Vec3 targetRight = baseCenter + right * radius;
		Vec3 targetUp = baseCenter + up * radius;
		Vec3 targetLeft = baseCenter - right * radius;
		Vec3 targetDown = baseCenter - up * radius;

		DrawLine(position, baseCenter, color);
		DrawLine(position, targetRight, color);
		DrawLine(position, targetUp, color);
		DrawLine(position, targetLeft, color);
		DrawLine(position, targetDown, color);

		DrawCircle(baseCenter, -forward, radius, color);
	}

	void FrameGraphControl::CreateFrameGraph(Camera* camera,GraphicsDevice* device, Renderer* renderer, Vec2 targetSize, bool handleWindowResize)
	{
		m_FrameGraphBuilder = new FrameGraphBuilderHelper();
		//here we will use frame graph builder to create frame graphs
		//and we will take info of what kind of graph we need from render settings
		//or use can provide his own frame graph in the future
		m_FrameGraphBuilder->Init(device, renderer);

		U32 startWidth = targetSize.x;
		U32 startHeight = targetSize.y;

		///////////////Depth node
		FrameGraphResourceOutputCreation depthPrepassOutput;
		depthPrepassOutput.name = "Depth";
		depthPrepassOutput.type = FrameGraphResourceType::Attachment;
		depthPrepassOutput.resource_info.texture.format = device->GetDepthFormat();
		depthPrepassOutput.resource_info.texture.width = startWidth;
		depthPrepassOutput.resource_info.texture.height = startHeight;
		depthPrepassOutput.resource_info.texture.depth = 1;
		depthPrepassOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;
		depthPrepassOutput.resource_info.texture.usage = ImageUsageFlag::DEPTH_STENCIL_ATTACHMENT;

		m_FrameGraphBuilder->BeginNewNode("DepthPrepass");
		m_FrameGraphBuilder->AddOutput(depthPrepassOutput);
		m_FrameGraphBuilder->SetRenderPassControl(RegisterRenderPassControl(new VulkanDepthPrepassRenderPass((VulkanRenderer*)renderer, (VulkanDevice*)device, startWidth, startHeight, handleWindowResize)));
		m_FrameGraphBuilder->SetClearColor(Vec3(0.0f));
		m_FrameGraphBuilder->EndNode();
		//////////////////////////

		///////////////Depth to linear pass

		FrameGraphResourceInputCreation depthAsTextureInput;
		depthAsTextureInput.type = FrameGraphResourceType::Texture;
		depthAsTextureInput.name = "Depth";

		FrameGraphResourceOutputCreation depthLinearPrepassOutput;
		depthLinearPrepassOutput.name = "DepthLinear";
		depthLinearPrepassOutput.type = FrameGraphResourceType::Attachment;
		depthLinearPrepassOutput.resource_info.texture.format = Format::R8G8B8A8_UNORM;
		depthLinearPrepassOutput.resource_info.texture.width = startWidth;
		depthLinearPrepassOutput.resource_info.texture.height = startHeight;
		depthLinearPrepassOutput.resource_info.texture.depth = 1;
		depthLinearPrepassOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;
		depthLinearPrepassOutput.resource_info.texture.usage = ImageUsageFlag::COLOR_ATTACHMENT;

		m_FrameGraphBuilder->BeginNewNode("DepthLinearPrepass");
		m_FrameGraphBuilder->AddInput(depthAsTextureInput);
		m_FrameGraphBuilder->SetClearColor(Vec3(0.0f, 0.0f, 1.0f));
		m_FrameGraphBuilder->AddOutput(depthLinearPrepassOutput);
		m_FrameGraphBuilder->SetRenderPassControl(RegisterRenderPassControl(new VulkanDepthLinearPrepassRenderPass((VulkanRenderer*)renderer, (VulkanDevice*)device, startWidth, startHeight, handleWindowResize)));
		m_FrameGraphBuilder->EndNode();
		/////////////////////////////


		///////////////Lighting pass

		FrameGraphResourceInputCreation depthInput;
		depthInput.type = FrameGraphResourceType::Attachment;
		depthInput.name = "Depth";

		FrameGraphResourceOutputCreation lightingOutput;
		lightingOutput.name = "Lighting";
		lightingOutput.type = FrameGraphResourceType::Attachment;
		lightingOutput.resource_info.texture.format = Format::R16G16B16A16_FLOAT;//VK_FORMAT_R16G16B16A16_SFLOAT
		lightingOutput.resource_info.texture.width = startWidth;
		lightingOutput.resource_info.texture.height = startHeight;
		lightingOutput.resource_info.texture.depth = 1;
		lightingOutput.resource_info.texture.loadOp = AttachmentLoadOp::CLEAR;
		lightingOutput.resource_info.texture.usage = ImageUsageFlag::COLOR_ATTACHMENT;

		auto lightingPass = new VulkanDefaultForwardLightingRenderPass((VulkanRenderer*)renderer, (VulkanDevice*)device, startWidth, startHeight, handleWindowResize);
		m_FrameGraphBuilder->BeginNewNode("LightingPass");
		m_FrameGraphBuilder->AddInput(depthInput);
		m_FrameGraphBuilder->AddOutput(lightingOutput);
		m_FrameGraphBuilder->SetClearColor(Vec3(0.0f, 0.0f, 0.0f));
		m_FrameGraphBuilder->SetRenderPassControl(RegisterRenderPassControl(lightingPass));
		m_FrameGraphBuilder->EndNode();
		//////////////////////////

		///////////////Transparent Lighting pass

		FrameGraphResourceInputCreation depthInputTransparent;
		depthInputTransparent.type = FrameGraphResourceType::Attachment;
		depthInputTransparent.name = "Depth";

		FrameGraphResourceInputCreation lightingInputTransparent;
		lightingInputTransparent.type = FrameGraphResourceType::Attachment;
		lightingInputTransparent.name = "Lighting";
		//lightingInputTransparent.resource_info.texture.format = Format::R16G16B16A16_FLOAT;


		auto lightingPassTransparent = new VulkanDefaultForwardTransparentLightingRenderPass((VulkanRenderer*)renderer, (VulkanDevice*)device, startWidth, startHeight, handleWindowResize);

		m_FrameGraphBuilder->BeginNewNode("LightingPassTransparent");
		m_FrameGraphBuilder->AddInput(depthInputTransparent);
		m_FrameGraphBuilder->AddInput(lightingInputTransparent);
		m_FrameGraphBuilder->SetRenderPassControl(RegisterRenderPassControl(lightingPassTransparent));
		m_FrameGraphBuilder->EndNode();


		//////////////////

		/////////////////////////Debug Render Pass

		FrameGraphResourceInputCreation depthInputDebug;
		depthInputDebug.type = FrameGraphResourceType::Attachment;
		depthInputDebug.name = "Depth";

		FrameGraphResourceInputCreation lightingInputDebug;
		lightingInputDebug.type = FrameGraphResourceType::Attachment;
		lightingInputDebug.name = "Lighting";

		auto debugDrawPass = new VulkanDebugDrawRenderPass((VulkanRenderer*)renderer, (VulkanDevice*)device, startWidth, startHeight, handleWindowResize);

		m_FrameGraphBuilder->BeginNewNode("DebugDrawPass");
		m_FrameGraphBuilder->AddInput(depthInputDebug);
		m_FrameGraphBuilder->AddInput(lightingInputDebug);
		m_FrameGraphBuilder->SetRenderPassControl(RegisterRenderPassControl(debugDrawPass));
		m_FrameGraphBuilder->EndNode();

		m_FrameGraphBuilder->SetFrameGraphBuildCamera(camera);


		////////////////
		m_FrameGraph = new FrameGraph();
		m_FrameGraph->Init(camera,device, renderer);
		m_FrameGraphBuilder->Build(m_FrameGraph);
	}

	void FrameGraphControl::ReleaseFrameGraph()
	{
		m_FrameGraphBuilder->Release();
		m_FrameGraph->Release();
		delete m_FrameGraphBuilder;
		delete m_FrameGraph;
		m_FrameGraphBuilder = nullptr;
		m_FrameGraph = nullptr;
	}

	RenderPassControl* FrameGraphControl::RegisterRenderPassControl(RenderPassControl* control)
	{
		m_RenderPassControls.push_back(control);
		return control;
	}

	void FrameGraphControl::UnRegisterRenderPassControl(RenderPassControl* control)
	{
		auto it = std::find(m_RenderPassControls.begin(), m_RenderPassControls.end(), control);
		if (it != m_RenderPassControls.end())
		{
			m_RenderPassControls.erase(it);
		}
	}

	void* FrameGraphControl::GetFrameGraphFinalResult()
	{
		return m_FrameGraph->GetFinalRenderPassResult();
	}

	RenderPassControl* FrameGraphControl::GetRenderPassControlByType(RenderPassType type) const
	{
		return m_FrameGraph->GetRenderPassControlByType(type);
	}


	bool FrameGraphControl::HasRenderPass(RenderPassType type)
	{
		for (auto& control : m_RenderPassControls)
		{
			if (control->GetType() == type)
			{
				return true;
			}
		}
		return false;
	}

	void FrameGraphControl::OnResize(U32 width, U32 height, bool windowSizeChanged)
	{
		m_FrameGraph->ResizeRenderPasses(width, height, windowSizeChanged);
	}

	void FrameGraphControl::ResizeRenderPassType(RenderPassType type, U32 width, U32 height)
	{
		for (auto& control : m_RenderPassControls)
		{
			if (control->GetType() == type)
			{
				control->Resize(width, height);
				return;
			}
		}
		LOG_BE_ERROR("Render pass control for type " + std::to_string((U32)type) + " not found");
	}

	void FrameGraphControl::Init()
	{
		m_FrameGraph->AfterSystemInit();
	}

	void FrameGraphControl::BeforeFrameGraphExecute(CommandBuffer* cmdBuffer)
	{
		m_FrameGraph->BeforeFrameGraphExecute(cmdBuffer);
	}

	void FrameGraphControl::AfterFrameGraphExecute(CommandBuffer* cmdBuffer)
	{
		m_FrameGraph->AfterFrameGraphExecute(cmdBuffer);
	}

	void FrameGraphControl::Execute(GraphicsDevice* device, CommandBuffer* cmdBuffer)
	{
		if (m_FrameGraph == nullptr)
		{
			LOG_BE_ERROR("Frame graph is not initialized");
			return;
		}
		m_FrameGraph->Execute(device, cmdBuffer);
	}

	Camera* FrameGraphControl::GetCamera() const
	{
		return m_FrameGraph->GetCamera();
	}

	FinalFrameGraphNodeInfo FrameGraphControl::GetFinalFrameGraphNodeInfo() const
	{
		FinalFrameGraphNodeInfo info;
		info.finalTexture = m_FrameGraph->GetFinalTexture();
		info.finalDepthTexture = m_FrameGraph->GetFinalDepthTexture();
		info.finalRenderPass = m_FrameGraph->GetFinalRenderPassHandle();
		return info;
	}

	void FrameGraphControl::SetDebugRenderFlag(DebugRenderingFlag flag, bool enable)
	{
		if (m_FrameGraph)
		{
			m_FrameGraph->SetDebugRenderFlag(flag, enable);
		}
		else
		{
			LOG_BE_ERROR("Frame graph is not initialized, cannot set debug render flag");
		}
	}

	void FrameGraphControl::SetDebugRenderFlagsGroup(U32 flags)
	{
		if (m_FrameGraph)
		{
			if (flags & DebugRenderingFlag::MaterialIndexAsColor)
			{
				m_FrameGraph->SetDebugRenderFlag(DebugRenderingFlag::MaterialIndexAsColor, true);
			}
		}
		else
		{
			LOG_BE_ERROR("Frame graph is not initialized, cannot set debug render flags group");
		}
	}

}
