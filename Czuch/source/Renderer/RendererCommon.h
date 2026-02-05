#pragma once
#include "Graphics.h"
#include "Core/Math.h"
#include "DebugDraw.h"
#include "Subsystems/Scenes/Components/CameraComponent.h"

namespace Czuch
{
    class GraphicsDevice;
	struct MeshComponent;
	class MeshRendererComponent;
	class LightComponent;
	struct FrameGraphBuilderHelper;
	struct FrameGraph;
	class Renderer;
	class RenderContext;
	class IScene;
	class RenderPassControl;
	class CommandBuffer;

	struct RenderContextFillParams
	{
		MaterialInstanceHandle forcedMaterial;
		RenderPassType renderPassType;
		bool forceMaterialForAll = false;
		bool ignoreTransparent = false;
		bool isLightPass = false;
		bool isDirectionalShadowPass = false;
	};

	struct RenderContextControl
	{
		RenderContext *renderContext;
		RenderPassType renderPassType;

		bool SupportsRenderPass(RenderPassType type) const;
	};

	struct CZUCH_API RenderObjectInfo
	{
		TransformComponent *transform;
		MeshComponent *mesh;
		MeshRendererComponent *meshRenderer;
	};

	struct CZUCH_API LightObjectInfo
	{
		LightData lightData;
		TransformComponent *transform;
		LightComponent *light;
	};

	struct CZUCH_API RenderObjectsContainer
	{
		Array<RenderObjectInfo> allObjects;
		Array<LightObjectInfo> allLights;
		Array<RenderObjectGPUData> *renderObjectsData;
		BufferHandle currentRenderObjectsBuffer;
		AABB visibleObjectsAABB;

		void Clear()
		{
			allObjects.clear();
			allLights.clear();
			currentRenderObjectsBuffer = BufferHandle();
			renderObjectsData = nullptr;
		}
	};

	struct CZUCH_API RenderObjectInstance
	{
		Mat4x4 localToClipSpaceTransformation;
		glm::ivec4 paramData;
		Vec3 worldPosition;
		MeshHandle mesh;
		MaterialInstanceHandle overrideMaterial;
		int passIndex;

		RenderObjectInstance() = default;

		bool IsValid() const
		{
			return HANDLE_IS_VALID(mesh);
		}

		void UpdateSceneDataIfRequired(GraphicsDevice *device, SceneDataBuffers &sceneDataBuffers, RenderContextFillParams &fillParams) const;

		RenderObjectInstance(RenderObjectInstance &&other) noexcept = default;
		RenderObjectInstance &operator=(RenderObjectInstance &&other) noexcept = default;

		RenderObjectInstance(const RenderObjectInstance &source) = default;
		RenderObjectInstance &operator=(const RenderObjectInstance &) = default;
	};

	struct CZUCH_API RenderContextCreateInfo
	{
		RenderLayer renderLayer;
		RenderType renderType;
		int sortingOrder;
		bool autoClearBeforeRender;

		RenderContextCreateInfo()
		{
			renderLayer = RenderLayer::LAYER_0;
			renderType = RenderType::General;
			sortingOrder = 0;
			autoClearBeforeRender = true;
		}
	};


    struct DrawIndexedIndirectCommand
	{
		U32 indexCount;
		U32 instanceCount;
		U32 firstIndex;
		I32 vertexOffset;
		U32 firstInstance;
	};
}