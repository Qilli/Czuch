#pragma once
#include"Graphics.h"
#include"Core/Math.h"

namespace Czuch
{
	class GraphicsDevice;
	struct TransformComponent;
	struct MeshComponent;
	struct MeshRendererComponent;
	struct LightComponent;
	struct Camera;
	class RenderContext;
	class IScene;

	struct RenderContextFillParams
	{
		MaterialInstanceHandle forcedMaterial;
		RenderPassType renderPassType;
		bool forceMaterialForAll;
		bool ignoreTransparent = false;
	};

	struct RenderContextControl
	{
		RenderContext* renderContext;
		RenderPassType renderPassType;

		bool SupportsRenderPass(RenderPassType type) const;
	};

	struct CZUCH_API RenderObjectInfo
	{
		TransformComponent* transform;
		MeshComponent* mesh;
		MeshRendererComponent* meshRenderer;
	};

	struct CZUCH_API LightObjectInfo
	{
		LightData lightData;
		TransformComponent* transform;
		LightComponent* light;
	};

	struct CZUCH_API RenderObjectsContainer
	{
		Array<RenderObjectInfo> allObjects;
		Array<LightObjectInfo> allLights;

		void Clear()
		{
			allObjects.clear();
			allLights.clear();
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

		void UpdateSceneDataIfRequired(GraphicsDevice* device, SceneDataBuffers& sceneDataBuffers ,RenderContextFillParams& fillParams) const;

		RenderObjectInstance(RenderObjectInstance&& other) noexcept = default;
		RenderObjectInstance& operator=(RenderObjectInstance&& other) noexcept = default;

		RenderObjectInstance(const RenderObjectInstance& source) = default;
		RenderObjectInstance& operator=(const RenderObjectInstance&) = default;
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

	struct SceneCameraRenderingControl
	{
		Camera* camera;
		SceneData data;
		BufferDesc bufferDesc;
		BufferDesc lightsBufferDesc;
		BufferDesc tilesBufferDesc;
		BufferDesc lightsListBufferDesc;
		BufferDesc renderObjectsBufferDesc;
		BufferHandle buffer[MAX_FRAMES_IN_FLIGHT];
		BufferHandle lightsBuffer[MAX_FRAMES_IN_FLIGHT];
		BufferHandle tilesBuffer[MAX_FRAMES_IN_FLIGHT];
		BufferHandle lightsListBuffer[MAX_FRAMES_IN_FLIGHT];
		BufferHandle renderObjectsBuffer[MAX_FRAMES_IN_FLIGHT];

		U32 tiles_in_width;
		U32 tiles_in_height;
		U32 tiles_count;
		U32 lastLightsCount;

		Array<U32> lightsIndexList;;
		Array<LightData> lightsData;
		Array<RenderObjectGPUData> renderObjectsData;
		TilesDataContainer tilesDataContainer;
		bool lightsChanged = true;

		void Init(GraphicsDevice* device, Camera* cam);
		void OnSceneActive(GraphicsDevice* device);
		void Release(GraphicsDevice* device);
		void InitTilesBuffer(GraphicsDevice* device, bool resize, U32 width, U32 height);
		bool FillTilesWithLights(GraphicsDevice* device, const Array<LightObjectInfo>& allLight, U32 frame);
		/// <summary>
		/// Fill buffer with render objects data(materials info, transforms). If returns true thats mean we need to update materials for new buffer size
		/// </summary>
		/// <param name="device"></param>
		/// <param name="allObjects"></param>
		/// <param name="frame"></param>
		bool FillRenderObjectsData(GraphicsDevice* device, const RenderObjectsContainer& allObjects, U32 frame);
		/// <summary>
		/// Init render objects buffer, resize if needed
		/// </summary>
		/// <param name="device"></param>
		/// <param name="resize"></param>
		/// <param name="frame"></param>
		void InitRenderObjectsBuffer(GraphicsDevice* device, bool resize,U32 size);
		void UpdateMaterialsLightsInfo();
		void UpdateMaterialsRenderObjectsInfo();

		SceneDataBuffers GetSceneDataBuffers(U32 frame);
		void UpdateSceneDataBuffers(IScene* scene, GraphicsDevice* device,RenderObjectsContainer& visibleObjects, U32 frame, DeletionQueue& deletionQueue);
	};


	struct SceneCameraRendering
	{
		SceneCameraRenderingControl cameraControl;
		IScene* activeScene;

		void Release(GraphicsDevice* device);
		void OnSceneActive(Camera* camera,GraphicsDevice* device, IScene* scene);

	};


	struct SceneCameraControl
	{
		Camera* camera;
		SceneCameraRendering cameraRendering;
		bool isPrimaryCamera;
		Array<RenderContextControl> renderContexts;
		RenderObjectsContainer visibleRenderObjects;

		/// <summary>
		/// Release is called when we are leaving the scene or when the scene is destroyed or when camera is removed
		/// We clear all render data for current camera
		/// </summary>
		void Release(GraphicsDevice* device);
		/// <summary>
		///This method is called when the scene is activated. Here we need to prepare the camera control for rendering
		/// </summary>
		/// <param name="device"></param>
		/// <param name="scene"></param>
		void OnSceneActive(GraphicsDevice* device, IScene* scene);
		void OnResize(GraphicsDevice* device, U32 width, U32 height, bool windowSizeChanged);
		SceneDataBuffers GetSceneDataBuffers(U32 frame);
		void UpdateSceneDataBuffers(GraphicsDevice* device, U32 frame, DeletionQueue& deletionQueue);

		RenderContext* GetRenderContext(RenderPassType type,bool createIfNotExist=true);
		void AddRenderContext(RenderContextCreateInfo ctx, RenderPassType type);
		bool IsPrimaryCamera() const { return isPrimaryCamera; }
		RenderContext* FillRenderList(GraphicsDevice* device, RenderContextFillParams& fillParams);
		/// <summary>
		/// make contexts dirty so we can update them at the beginning of the next frame
		/// </summary>
		void OnPostRender();

		void UpdateVisibleObjects(RenderObjectsContainer& allObjects);

	private:
		void ReleaseRenderContexts();
	};


	class CZUCH_API RenderContext
	{
	public:
		RenderContext(RenderContextCreateInfo& createInfo) :m_Settings(createInfo) { m_RenderObjects.reserve(1000); m_IsDirty = true; }
		RenderContext() = default;
		inline void ClearRenderList() { m_RenderObjects.clear(); m_IsDirty = true; }
		inline void AddToRenderList(RenderObjectInstance& instance) {m_RenderObjects.push_back(std::move(instance)); }
		inline const Array<RenderObjectInstance>& GetRenderObjectsList() const { return m_RenderObjects; }
		inline bool IsAutoCleanEnabled() const { return m_Settings.autoClearBeforeRender; }
		inline RenderLayer GetRenderLayer() const { return m_Settings.renderLayer; }
		inline RenderType GetRenderType() const { return m_Settings.renderType; }
		inline int GetSortingOrder() const { return m_Settings.sortingOrder; }
		virtual void FillRenderList(GraphicsDevice* device, Camera* cam,RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams) = 0;
		inline bool IsDirty() const { return m_IsDirty; }
		virtual bool SupportRenderPass(RenderPassType type) const;
	protected:
		Array<RenderObjectInstance> m_RenderObjects;
		RenderContextCreateInfo m_Settings;
		bool m_IsDirty = false;
	};


	class CZUCH_API DefaultRenderContext : public RenderContext
	{
	public:
		DefaultRenderContext(RenderContextCreateInfo& createInfo) :RenderContext(createInfo) {}
		virtual void FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams) override;
		bool SupportRenderPass(RenderPassType type) const override;
	private:
		const U32 SUPPORTED_RENDER_PASSES_FLAGS =
			(U32)RenderPassType::ForwardLighting;
	};

	class CZUCH_API DefaultDepthPrePassRenderContext : public DefaultRenderContext
	{
	public:
		DefaultDepthPrePassRenderContext(RenderContextCreateInfo& createInfo) :DefaultRenderContext(createInfo) {}
		void FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams) override;
		bool SupportRenderPass(RenderPassType type) const override;
	private:
		const U32 SUPPORTED_RENDER_PASSES_FLAGS = (U32)RenderPassType::DepthPrePass | (U32)RenderPassType::DepthLinearPrePass;
	};

	class CZUCH_API DefaultTransparentRenderContext : public DefaultRenderContext
	{
	public:
		DefaultTransparentRenderContext(RenderContextCreateInfo& createInfo): DefaultRenderContext(createInfo) {}
		virtual void FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams) override;
		bool SupportRenderPass(RenderPassType type) const override;
	protected:
		void SortRenderObjects(Camera* cam);
	private:
		const U32 SUPPORTED_RENDER_PASSES_FLAGS =
			(U32)RenderPassType::ForwardLightingTransparent;
	};

	class CZUCH_API DebugRenderContext : public RenderContext
	{
	public:
		DebugRenderContext(RenderContextCreateInfo& createInfo) :RenderContext(createInfo) {}
		virtual void FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams) override;
		bool SupportRenderPass(RenderPassType type) const override;
	private:
		const U32 SUPPORTED_RENDER_PASSES_FLAGS =
			(U32)RenderPassType::DebugDraw;
	};

}

