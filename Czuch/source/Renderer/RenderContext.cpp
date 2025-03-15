#include "czpch.h"
#include "RenderContext.h"
#include "GraphicsDevice.h"
#include "Subsystems/Scenes/Components/TransformComponent.h"
#include "Subsystems/Scenes/Components/MeshComponent.h"
#include "Subsystems/Scenes/Components/MeshRendererComponent.h"
#include "Subsystems/Scenes/Components/CameraComponent.h"

namespace Czuch
{
    void RenderObjectInstance::UpdateSceneDataIfRequired(GraphicsDevice* device,BufferHandle buffer,RenderContextFillParams& fillParams) const
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
                m->params[passIndex].shaderParamsDesc[0].descriptors[0].resource = buffer.handle;
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
    }

    void DefaultTransparentRenderContext::FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams)
    {
		DefaultRenderContext::FillRenderList(device, cam, allObjects, fillParams);

		//sort by distance
		SortRenderObjects(cam);
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
    }
}
