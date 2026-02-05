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
#include "Subsystems/Scenes/Components/HeaderComponent.h"
#include "Renderer/RenderPassControl.h"
#include "Renderer/FrameGraph/FrameGraphBuilderHelper.h"

#include "Renderer/Vulkan/RenderPass/VulkanDepthPrepassRenderPass.h"
#include "Renderer/Vulkan/RenderPass/VulkanDefaultForwardLightingRenderPass.h"
#include "Renderer/Vulkan/RenderPass/VulkanDepthLinearPrepassRenderPass.h"
#include "Renderer/Vulkan/RenderPass/VulkanDefaultForwardTransparentLightingRenderPass.h"
#include "Renderer/Vulkan/RenderPass/VulkanDebugDrawRenderPass.h"
#include "Renderer/Vulkan/RenderPass/VulkanDirectionalShadowMapRenderPass.h"
#include "SceneCameraControl.h"

namespace Czuch
{
	void DefaultRenderContext::FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams)
	{
		// ClearRenderList(); // Not needeed probably, we are clearing render lists at the end of frame if necessary

		U32 id = 0;
		U32 counter = 0;

		for (auto &obj : allObjects.allObjects)
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
				renderObjectInstance.paramData.x = counter;
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

	void DefaultTransparentRenderContext::FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams)
	{
		DefaultRenderContext::FillRenderList(device, cam, allObjects, fillParams);

		// sort by distance
		SortRenderObjects(cam);
	}

	bool DefaultTransparentRenderContext::SupportRenderPass(RenderPassType type) const
	{
		return (U32)type & SUPPORTED_RENDER_PASSES_FLAGS;
	}

	void DefaultTransparentRenderContext::SortRenderObjects(Camera *cam)
	{
		Vec3 cameraPosition = cam->GetViewMatrix()[3];

		std::sort(m_RenderObjects.begin(), m_RenderObjects.end(), [cameraPosition](const RenderObjectInstance &a, const RenderObjectInstance &b)
				  {
				float distanceA = glm::length(a.worldPosition - cameraPosition);
				float distanceB = glm::length(b.worldPosition - cameraPosition);
				return distanceA > distanceB; });
	}

	void DebugRenderContext::FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams)
	{
		// ClearRenderList(); // Not needeed probably, we are clearing render lists at the end of frame if necessary

		MeshHandle meshH;
		meshH.handle = 4998;
		MaterialInstanceHandle materialH;
		materialH = DefaultAssets::DEBUG_DRAW_LIGHT_MATERIAL_INSTANCE;

		for (auto &lightObj : allObjects.allLights)
		{
			RenderObjectInstance renderObjectInstance;
			Mat4x4 localToWorld = lightObj.transform->GetLocalToWorld();
			// leave only position
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

	void DefaultDepthPrePassRenderContext::FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams)
	{
		// ClearRenderList(); // Not needeed probably, we are clearing render lists at the end of frame if necessary

		U32 id = 0;

		for (auto &obj : allObjects.allObjects)
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

	void DefaultDirectionalShadowMapRenderContext::FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams)
	{
		auto material = fillParams.forcedMaterial;
		auto materialInstance = device->AccessMaterialInstance(material);
		if (!materialInstance)
		{
			LOG_BE_ERROR("Material instance is null");
			return;
		}

		I32 index = device->AccessMaterial(materialInstance->handle)->GetRenderPassIndexForType(fillParams.renderPassType);

		if (index < 0)
		{
			LOG_BE_ERROR("[DirectionalShadowMapRenderContext] Selected material do not support shadow pass");
			return;
		}

		for (auto &obj : allObjects.allObjects)
		{
			auto currentMaterial = obj.meshRenderer->GetOverrideMaterial();
			auto currentMaterialInstance = device->AccessMaterialInstance(currentMaterial);

			auto ignoreTransparent = fillParams.ignoreTransparent;
			auto isTransparent = currentMaterialInstance->IsTransparent();
			auto isShadowCaster = obj.meshRenderer->IsShadowCaster();

			if ((ignoreTransparent && isTransparent) || !isShadowCaster)
			{
				continue;
			}

			RenderObjectInstance renderObjectInstance;

			renderObjectInstance.localToClipSpaceTransformation = cam->GetViewProjectionMatrix() * obj.transform->GetLocalToWorld();
			renderObjectInstance.worldPosition = obj.transform->GetWorldPosition();
			renderObjectInstance.paramData.x = 0;
			renderObjectInstance.mesh = obj.mesh->GetMesh();
			renderObjectInstance.overrideMaterial = material;
			renderObjectInstance.passIndex = index;
			AddToRenderList(renderObjectInstance);
		}
		m_IsDirty = false;
	}

	bool DefaultDirectionalShadowMapRenderContext::SupportRenderPass(RenderPassType type) const
	{
		return (U32)type & SUPPORTED_RENDER_PASSES_FLAGS;
	}
	Mat4x4 CreateLightView(const Vec3 &eye, const Vec3 &forward)
	{
		// 1. Forward is +Z (Left Handed)
		Vec3 f = glm::normalize(forward);

		// 2. Compute Right axis
		// If forward is close to world UP (0,1,0), pick a different temp up vector
		Vec3 upRef = Vec3(0.0f, 1.0f, 0.0f);
		if (glm::abs(glm::dot(f, upRef)) > 0.99f)
		{
			upRef = Vec3(0.0f, 0.0f, 1.0f); // Use Z as temp up
		}

		Vec3 r = glm::normalize(glm::cross(upRef, f)); // Left Handed Cross: Up x Forward = Right

		// 3. Compute actual Up axis
		Vec3 u = glm::cross(f, r); // Forward x Right = Up

		// 4. Construct matrices
		// Rotation (Transposed basis vectors)
		Mat4x4 R(1.0f);
		R[0][0] = r.x;
		R[1][0] = r.y;
		R[2][0] = r.z;
		R[0][1] = u.x;
		R[1][1] = u.y;
		R[2][1] = u.z;
		R[0][2] = f.x;
		R[1][2] = f.y;
		R[2][2] = f.z;

		// Translation
		Mat4x4 T(1.0f);
		T[3][0] = -eye.x;
		T[3][1] = -eye.y;
		T[3][2] = -eye.z;

		return R * T;
	}

	void CameraLightsInfo::Rebuild(IScene *scene, FrameGraphControl *frameGraph, Camera *camera, IDebugDrawBuilder *debugDraw)
	{
		auto &allLights = scene->GetAllLightObjects();
		// first find first directional light
		for (auto &light : allLights)
		{
			if (light.light->GetLightType() == LightType::Directional)
			{
				this->directionalLight = light.light;
				// set texture global id for directional shadow map
				this->directionalLight->SetShadowMapTextureHandle(frameGraph->GetRenderPassControlByType(RenderPassType::DirectionalShadowMap)->GetMainAttachmentTextureHandle());
				break;
			}
		}

		if (directionalLight != nullptr && directionalLight->GetShadowMapTextureHandle().globalIndex != -1)
		{
			if (directionalCameraShadowCasterTransform == nullptr)
			{
				// we will use fake invisible objects to control camerea position for dirctional shadow caster
				entt::entity newObj = scene->CreateEmptyEntity("DirectionalShadowCasterPosition");
				Entity entity(newObj, scene);
				directionalCameraShadowCasterTransform = &entity.GetComponent<TransformComponent>();

				// make it invisible in editor hierarchy
				auto &headerComponent = entity.GetComponent<HeaderComponent>();
				headerComponent.SetVisibleInEditorHierarchy(true);
			}

			// compute world frustum from main camera
			WorldFrustum mainCameraFrustum = Math::CalculateFrustumForShadowDistance(camera->GetTransform()->GetLocalToWorld(), camera->GetFov(), camera->GetAspectRatio(), camera->GetNearPlane(), directionalLight->GetShadowDistance());

			// compute shadow caster position based on main camera frustum
			auto center = mainCameraFrustum.GetCenter();
			auto radius = directionalLight->GetShadowDistance() * 0.5f;

			// debugDraw->DrawLinesSphere(center, radius, Colors::Green);

			auto transformComp = directionalLight->GetEntity().GetComponent<TransformComponent>();
			Vec3 direction = transformComp.GetForward();
			direction = glm::normalize(direction);

			U32 shadowMapResolution = EngineRoot::GetEngineSettings().directionalShadowMapResolution;
			Vec3 shadowCasterPosition = center;

			// 1. Calculate the size of one texel in world units
			float worldUnitsPerTexel = (2.0f * radius) / (float)shadowMapResolution;

			// 2. Transform the center position to Light Space (using the light's rotation)
			Mat4x4 lightView = CreateLightView(Vec3(0, 0, 0), direction); // View matrix without translation
			Vec3 lightSpacePos = Vec3(lightView * Vec4(shadowCasterPosition, 1.0f));

			// 3. Snap the X and Y coordinates to the nearest texel
			lightSpacePos.x = floor(lightSpacePos.x / worldUnitsPerTexel) * worldUnitsPerTexel;
			lightSpacePos.y = floor(lightSpacePos.y / worldUnitsPerTexel) * worldUnitsPerTexel;

			// 4. Transform back to World Space
			Mat4x4 lightViewInv = glm::inverse(lightView);
			shadowCasterPosition = Vec3(lightViewInv * Vec4(lightSpacePos, 1.0f));

			float texelSize = worldUnitsPerTexel;
			float zMin = glm::floor((-radius * 2.0f) / texelSize) * texelSize;
			float zMax = -zMin;

			float width = glm::floor((radius * 2.0f) / texelSize) * texelSize;
			float height = width;
			// set position from input for our directional camera special transform + rotation
			directionalCameraShadowCasterTransform->SetLocalPosition(shadowCasterPosition);
			auto lookAt = shadowCasterPosition - direction;
			directionalCameraShadowCasterTransform->LookAt(lookAt);
			directionalCameraShadowCasterTransform->ForceUpdateLocalTransform();

			// now we need to create camera for our light
			// directional light will have orto camera
			// position, look at, ortho width, ortho height
			directionalLightCamera = Camera::CreateOrthoCamera(directionalCameraShadowCasterTransform, zMin, zMax, width, height);

			directionalLight->SetDirectionalLightViewProj(directionalLightCamera.GetViewProjectionMatrix());
		}
	}

	void CameraLightsInfo::Reset()
	{
		this->directionalLight = nullptr;
		this->lightsCount = 0;

		for (U32 a = 0; a < MAX_LIGHTS_WITH_SHADOWS; a++)
		{
			this->spotAndPointLights[a] = nullptr;
		}
	}

}
