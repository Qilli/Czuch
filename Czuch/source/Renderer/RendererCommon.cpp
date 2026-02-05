#include "czpch.h"
#include"RendererCommon.h"
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

namespace Czuch
{
    void RenderObjectInstance::UpdateSceneDataIfRequired(GraphicsDevice *device, SceneDataBuffers &sceneDataBuffers, RenderContextFillParams &fillParams) const
	{
		if (IsValid())
		{
			MaterialInstance *m = nullptr;
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
				// m->params[passIndex].shaderParamsDesc[0].descriptors[0].resource = sceneDataBuffers.sceneDataBuffer;
				if (m->params[passIndex].setsCount > 0)
				{
					m->params[passIndex].SetUniformBuffer(0, sceneDataBuffers.sceneDataBuffer, 0); // scene data buffer
				}

				if (fillParams.renderPassType == RenderPassType::ForwardLighting || fillParams.renderPassType == RenderPassType::ForwardLightingTransparent)
				{
					// m->params[passIndex].shaderParamsDesc[0].descriptors[1].resource = sceneDataBuffers.renderObjectsBuffer; //render objects buffer
					m->params[passIndex].SetStorageBuffer(0, sceneDataBuffers.renderObjectsBuffer, 1); // render objects buffer
					// m->params[passIndex].shaderParamsDesc[1].descriptors[0].resource = sceneDataBuffers.lightsDataBuffer; //lights data buffer
					m->params[passIndex].SetStorageBuffer(1, sceneDataBuffers.lightsDataBuffer, 0); // lights data buffer
					// m->params[passIndex].shaderParamsDesc[1].descriptors[1].resource = sceneDataBuffers.lightsIndexListBuffer; //lights index buffer
					m->params[passIndex].SetStorageBuffer(1, sceneDataBuffers.lightsIndexListBuffer, 1); // lights index buffer
					// m->params[passIndex].shaderParamsDesc[1].descriptors[2].resource = sceneDataBuffers.tilesDataBuffer; //tiles data buffer
					m->params[passIndex].SetStorageBuffer(1, sceneDataBuffers.tilesDataBuffer, 2); // tiles data buffer
				}
			}
		}
	}

}