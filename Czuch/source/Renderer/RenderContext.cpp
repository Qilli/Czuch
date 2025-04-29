#include "czpch.h"
#include "RenderContext.h"
#include "GraphicsDevice.h"
#include "Subsystems/Scenes/Components/TransformComponent.h"
#include "Subsystems/Scenes/Components/MeshComponent.h"
#include "Subsystems/Scenes/Components/MeshRendererComponent.h"
#include "Subsystems/Scenes/Components/CameraComponent.h"
#include "Subsystems/Scenes/Components/LightComponent.h"
#include "Subsystems/Assets/AssetsManager.h"

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
				m->params[passIndex].shaderParamsDesc[0].descriptors[0].resource = sceneDataBuffers.sceneDataBuffer;

				if (fillParams.renderPassType == RenderPassType::ForwardLighting || fillParams.renderPassType == RenderPassType::ForwardLightingTransparent)
				{
					m->params[passIndex].shaderParamsDesc[1].descriptors[0].resource = sceneDataBuffers.lightsDataBuffer; //lights data buffer
					m->params[passIndex].shaderParamsDesc[1].descriptors[1].resource = sceneDataBuffers.lightsIndexListBuffer; //lights index buffer
					m->params[passIndex].shaderParamsDesc[1].descriptors[2].resource = sceneDataBuffers.tilesDataBuffer; //tiles data buffer
				}

			}

		}
	}

	void DefaultRenderContext::FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams)
	{
		ClearRenderList();

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
				renderObjectInstance.localToWorldTransformation = obj.transform->GetLocalToWorld();
				renderObjectInstance.mesh = obj.mesh->GetMesh();
				renderObjectInstance.overrideMaterial = material;
				renderObjectInstance.passIndex = index;
				AddToRenderList(renderObjectInstance);
			}
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
				Vec3 translationA = glm::vec3(a.localToWorldTransformation[3]);
				Vec3 translationB = glm::vec3(b.localToWorldTransformation[3]);
				float distanceA = glm::length(translationA - cameraPosition);
				float distanceB = glm::length(translationB - cameraPosition);
				return distanceA > distanceB;
			});
	}

	void DebugRenderContext::FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams)
	{
		ClearRenderList();

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
			renderObjectInstance.localToWorldTransformation = localToWorld;
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
		visibleRenderObjects.Clear();
		ReleaseRenderContexts();
		cameraRendering.Release(device);
	}

	void SceneCameraControl::OnSceneActive(GraphicsDevice* device, IScene* scene)
	{
		cameraRendering.OnSceneActive(camera, device, scene);
	}

	void SceneCameraControl::OnResize(GraphicsDevice* device, U32 width, U32 height,bool windowSizeChanged)
	{
		cameraRendering.cameraControl.InitTilesBuffer(device, true, width, height);
	}

	SceneDataBuffers SceneCameraControl::GetSceneDataBuffers(U32 frame)
	{
		return cameraRendering.cameraControl.GetSceneDataBuffers(frame);
	}

	void SceneCameraControl::UpdateSceneDataBuffers(GraphicsDevice* device, U32 frame, DeletionQueue& deletionQueue)
	{
		cameraRendering.cameraControl.UpdateSceneDataBuffers(cameraRendering.activeScene, device, frame, deletionQueue);
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

	RenderContext* SceneCameraControl::FillRenderList(GraphicsDevice* device,RenderContextFillParams& fillParams)
	{
		auto context = GetRenderContext(fillParams.renderPassType, true);
		if (context->IsDirty())
		{
			context->FillRenderList(device, camera, visibleRenderObjects, fillParams);
		}
		return context;
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
	}

	void SceneCameraRendering::OnSceneActive(Camera* camera, GraphicsDevice* device, IScene* scene)
	{
		activeScene = scene;
		cameraControl.Init(device, camera);
		cameraControl.OnSceneActive(device);
	}

	void SceneCameraRenderingControl::Init(GraphicsDevice* device, Camera* cam)
	{
		lightsIndexList.reserve(MAX_LIGHTS_IN_SCENE * MAX_LIGHTS_IN_SCENE);
		tilesDataContainer.tilesData.reserve(MAX_LIGHTS_IN_SCENE * MAX_LIGHTS_IN_SCENE);
		lightsData.reserve(MAX_LIGHTS_IN_SCENE);
		camera = cam;
	}

	void SceneCameraRenderingControl::OnSceneActive(GraphicsDevice* device)
	{
		bufferDesc.persistentMapped = true;
		bufferDesc.elementsCount = 1;
		bufferDesc.bind_flags = BindFlag::UNIFORM_BUFFER;
		bufferDesc.size = sizeof(SceneData);
		bufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		lightsBufferDesc.persistentMapped = true;
		lightsBufferDesc.elementsCount = MAX_LIGHTS_IN_SCENE;
		lightsBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		lightsBufferDesc.size = sizeof(LightData) * MAX_LIGHTS_IN_SCENE;
		lightsBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		InitTilesBuffer(device, false, 0, 0);

		data.ambientColor = Vec4(1, 1, 1, 1);

		for (int a = 0; a < MAX_FRAMES_IN_FLIGHT; ++a)
		{
			device->Release(buffer[a]);
			INVALIDATE_HANDLE(buffer[a]);
			lightsBuffer[a] = device->CreateBuffer(&lightsBufferDesc);
		}
	}

	void SceneCameraRenderingControl::Release(GraphicsDevice* device)
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			device->Release(tilesBuffer[i]);
			device->Release(lightsListBuffer[i]);
			device->Release(lightsBuffer[i]);
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
		tilesBufferDesc.size = sizeof(LightsTileData)* tiles_count;
		tilesBufferDesc.usage = Usage::MEMORY_USAGE_CPU_TO_GPU;

		lightsListBufferDesc.persistentMapped = true;
		lightsListBufferDesc.elementsCount = 1;
		lightsListBufferDesc.bind_flags = BindFlag::STORAGE_BUFFER;
		lightsListBufferDesc.size = sizeof(U32)* MAX_LIGHTS_IN_TILE * tiles_count +sizeof(glm::ivec4);
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
			buffer[frame].handle,
			lightsBuffer[frame].handle,
			lightsListBuffer[frame].handle,
			tilesBuffer[frame].handle, };
	}

	void SceneCameraRenderingControl::UpdateSceneDataBuffers(IScene* scene, GraphicsDevice* device, U32 frame, DeletionQueue& deletionQueue)
	{
		buffer[frame] = device->CreateBuffer(&bufferDesc);
		deletionQueue.PushFunction([=, this]() {
			if (HANDLE_IS_VALID(buffer[frame]))
			{
				device->Release(buffer[frame]);
			}
			});

		if (scene != nullptr)
		{
			data.ambientColor = scene->GetAmbientColor();

			auto& lights = scene->GetAllLightObjects();
			if (FillTilesWithLights(device, lights, frame))
			{
				UpdateMaterialsLightsInfo();
			}
		}

		device->UploadDataToBuffer(buffer[frame], &data, sizeof(SceneData));
	}


	void SceneCameraRenderingControl::UpdateMaterialsLightsInfo()
	{
		AssetsManager::Get().UpdateLightingMaterialsLightInfo(MAX_LIGHTS_IN_SCENE, MAX_LIGHTS_IN_TILE * tiles_count, tiles_count);
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
		memcpy(lightsListData+sizeof(glm::ivec4), lightsIndexList.data(), sizeof(U32) * lightsIndexList.size() + sizeof(glm::ivec4));

		bool localLightsChanged = lightsChanged;
		lightsChanged = false;
		return localLightsChanged;
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
		ClearRenderList();

		for (auto& obj : allObjects.allObjects)
		{
			auto currentMaterial = obj.meshRenderer->GetOverrideMaterial();
			auto currentMaterialInstance = device->AccessMaterialInstance(currentMaterial);

			if (fillParams.ignoreTransparent && currentMaterialInstance->IsTransparent())
			{
				continue;
			}

			auto material =  fillParams.forcedMaterial;
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
				renderObjectInstance.localToWorldTransformation = obj.transform->GetLocalToWorld();
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

}
