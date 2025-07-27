#pragma once
#include"Graphics.h"
#include"Core/Math.h"
#include"DebugDraw.h"

namespace Czuch
{
	class GraphicsDevice;
	struct TransformComponent;
	struct MeshComponent;
	struct MeshRendererComponent;
	struct LightComponent;
	struct Camera;
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

		void UpdateSceneDataIfRequired(GraphicsDevice* device, SceneDataBuffers& sceneDataBuffers, RenderContextFillParams& fillParams) const;

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
		void InitRenderObjectsBuffer(GraphicsDevice* device, bool resize, U32 size);
		void UpdateMaterialsLightsInfo();
		void UpdateMaterialsRenderObjectsInfo();

		SceneDataBuffers GetSceneDataBuffers(U32 frame);
		void UpdateSceneDataBuffers(IScene* scene, GraphicsDevice* device, RenderObjectsContainer& visibleObjects, U32 frame, DeletionQueue& deletionQueue);
	};


	struct DrawIndexedIndirectCommand {
		U32 indexCount;
		U32 instanceCount;
		U32 firstIndex;
		I32 vertexOffset;
		U32 firstInstance;
	};


	class SceneCameraDebugRenderingControl : public IDebugDrawBuilder
	{
	private:
		BufferDesc m_LinesBufferDesc;
		BufferDesc m_VertexBufferLinesDesc;
		BufferDesc m_IndexBufferLinesDesc;
		BufferHandle m_VertexBufferLines;
		BufferHandle m_IndexBufferLines;
		BufferHandle m_LinesBuffer[MAX_FRAMES_IN_FLIGHT];
		IndirectDrawForCommandBufferData m_IndirectDrawDataLines;
		void CreateAndInitLinesBuffer(GraphicsDevice* device);
		void ReleaseLinesBuffer(GraphicsDevice* device);
	private:
		BufferDesc m_TrianglesBufferDesc;
		BufferDesc m_VertexBufferTrianglesDesc;
		BufferDesc m_IndexBufferTrianglesDesc;
		BufferHandle m_VertexBufferTriangles;
		BufferHandle m_IndexBufferTriangles;
		BufferHandle m_TrianglesBuffer[MAX_FRAMES_IN_FLIGHT];
		IndirectDrawForCommandBufferData m_IndirectDrawDataTriangles;
		void CreateAndInitTrianglesBuffer(GraphicsDevice* device);
		void ReleaseTrianglesBuffer(GraphicsDevice* device);
	private:
		BufferDesc m_PointsBufferDesc;
		BufferDesc m_VertexBufferPointsDesc;
		BufferHandle m_VertexBufferPoints;
		BufferHandle m_PointsBuffer[MAX_FRAMES_IN_FLIGHT];
		IndirectDrawForCommandBufferData m_IndirectDrawDataPoints;
		void CreateAndInitPointsBuffer(GraphicsDevice* device);
		void ReleasePointsBuffer(GraphicsDevice* device);
	private:
		BufferDesc m_CommandsBufferDesc;
		BufferDesc m_SceneBufferDesc;
		BufferHandle m_SceneBuffer[MAX_FRAMES_IN_FLIGHT];
		BufferHandle m_CommandsBuffer[MAX_FRAMES_IN_FLIGHT];
		SceneData m_SceneData;
	private:
		Array<LineInstanceData> m_Lines;
		Array<TriangleInstanceData> m_Triangles;
		Array<PointInstanceData> m_Points;
		Array<MeshInstanceData> m_Meshes;
		Array<DrawIndexedIndirectCommand> indirectCommands;
		Camera* m_Camera;
	public:
		void UpdateSceneDataBuffer(IScene* scene, GraphicsDevice* device, U32 frame);
		void OnSceneActive(Camera* cam, GraphicsDevice* device);
		void Release(GraphicsDevice* device);
		bool FillDebugBuffersData(GraphicsDevice* device, U32 frame);
		void UpdateDebugMaterialInfo();
		IndirectDrawForCommandBufferData& FillAndGetIndirectDrawDataForDebugLinesDrawing(MaterialInstanceHandle material, U32 frame);
		IndirectDrawForCommandBufferData& FillAndGetIndirectDrawDataForDebugTrianglesDrawing(MaterialInstanceHandle material, U32 frame);
		IndirectDrawForCommandBufferData& FillAndGetIndirectDrawDataForDebugPointsDrawing(MaterialInstanceHandle material, U32 frame);
	public:
		SceneCameraDebugRenderingControl()
		{
			m_Lines.reserve(MAX_LINES_IN_SCENE);
			m_Triangles.reserve(1000);
			m_Points.reserve(1000);
			m_Meshes.reserve(100);
			indirectCommands.resize(3);
		}

		void Clear()
		{
			m_Lines.clear();
			m_Triangles.clear();
			m_Points.clear();
			m_Meshes.clear();
		}
		/// <summary>
		///  Those methods can be used to draw debug lines, triangles, points and meshes
		/// every component gets access to IDebugDrawBuilder interface
		/// </summary>
		void DrawLine(const Vec3& start, const Vec3& end, const Color& color) override;
		void DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color) override;
		void DrawPoint(const Vec3& point, const Color& color, const float size) override;
		void DrawMesh(const AssetHandle mesh, const Mat4x4& transform, const Color& color) override;
		void DrawQuad(const Vec3& center, const Vec3& normal, float size, const Color& color) override;
		void DrawCircle(const Vec3& center, const Vec3& normal, float radius, const Color& color) override;
		void DrawLinesSphere(const Vec3& center, float radius, const Color& color) override;
		void DrawCone(const Vec3& position, const Vec3& direction, float range, float angle, const Color& color) override;
	};


	struct FinalFrameGraphNodeInfo
	{
		TextureHandle finalTexture;
		TextureHandle finalDepthTexture;
		RenderPassHandle finalRenderPass;
	};

	struct FrameGraphControl
	{
	private:
		FrameGraphBuilderHelper* m_FrameGraphBuilder;
		FrameGraph* m_FrameGraph;
		Array<RenderPassControl*> m_RenderPassControls;
	public:
		void CreateFrameGraph(Camera* camera,GraphicsDevice* device, Renderer* renderer, Vec2 targetSize, bool handleWindowResize);
		void ReleaseFrameGraph();
		RenderPassControl* RegisterRenderPassControl(RenderPassControl* control);
		void UnRegisterRenderPassControl(RenderPassControl* control);
		void* GetFrameGraphFinalResult();
		RenderPassControl* GetRenderPassControlByType(RenderPassType type) const;
		bool HasRenderPass(RenderPassType type);
		void OnResize(U32 width, U32 height, bool windowSizeChanged);
		void ResizeRenderPassType(RenderPassType type, U32 width, U32 height);
		void Init();
		/// <summary>
		/// Before new frame is renderered we call this method to prepare frame graph
		/// </summary>
		/// <param name="cmdBuffer"></param>
		void BeforeFrameGraphExecute(CommandBuffer* cmdBuffer);
		void AfterFrameGraphExecute(CommandBuffer* cmdBuffer);
		void Execute(GraphicsDevice* device, CommandBuffer* cmdBuffer);
		Camera* GetCamera() const;

		FinalFrameGraphNodeInfo GetFinalFrameGraphNodeInfo() const;
		void SetDebugRenderFlag(DebugRenderingFlag flag, bool enable);
		void SetDebugRenderFlagsGroup(U32 flags);
	};


	struct SceneCameraRendering
	{
		SceneCameraRenderingControl cameraControl;
		SceneCameraDebugRenderingControl debugCameraControl;
		IScene* activeScene;

		void Release(GraphicsDevice* device);
		void OnSceneActive(Camera* camera, GraphicsDevice* device, IScene* scene);
	};


	struct SceneCameraControl
	{
		SceneCameraControl()
		{
			camera = nullptr;
			currentScene = nullptr;
			isPrimaryCamera = false;
		}

		Camera* camera;
		IScene* currentScene;
		SceneCameraRendering cameraRendering;
		bool isPrimaryCamera;
		Array<RenderContextControl> renderContexts;
		RenderObjectsContainer visibleRenderObjects;
		FrameGraphControl frameGraphControl;
		/// <summary>
		/// Release is called when we are leaving the scene or when the scene is destroyed or when camera is removed
		/// We clear all render data for current camera
		/// </summary>
		void Release(GraphicsDevice* device);
		/// <summary>
		///This method is called when the scene is activated. Here we need to prepare the camera control for rendering
		/// </summary>
		/// <param name="renderer"></param>
		/// <param name="device"></param>
		/// <param name="scene"></param>
		void OnSceneActive(Renderer* renderer, GraphicsDevice* device, IScene* scene);
		void AfterSceneActive();
		void OnResize(GraphicsDevice* device, U32 width, U32 height, bool windowSizeChanged);
		SceneDataBuffers GetSceneDataBuffers(U32 frame);
		void UpdateSceneDataBuffers(GraphicsDevice* device, U32 frame, DeletionQueue& deletionQueue);

		RenderContext* GetRenderContext(RenderPassType type, bool createIfNotExist = true);
		void AddRenderContext(RenderContextCreateInfo ctx, RenderPassType type);
		bool IsPrimaryCamera() const { return isPrimaryCamera; }
		RenderContext* FillRenderList(GraphicsDevice* device, RenderContextFillParams& fillParams);
		void FillDebugDrawElements(GraphicsDevice* device, RenderContextFillParams& fillParams);
		IndirectDrawForCommandBufferData& GetIndirectDrawDataForDebugDrawingLines(RenderContextFillParams& fillParams, U32 frame);
		IndirectDrawForCommandBufferData& GetIndirectDrawDataForDebugDrawingTriangles(RenderContextFillParams& fillParams, U32 frame);
		IndirectDrawForCommandBufferData& GetIndirectDrawDataForDebugDrawingPoints(RenderContextFillParams& fillParams, U32 frame);
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
		inline void AddToRenderList(RenderObjectInstance& instance) { m_RenderObjects.push_back(std::move(instance)); }
		inline const Array<RenderObjectInstance>& GetRenderObjectsList() const { return m_RenderObjects; }
		inline bool IsAutoCleanEnabled() const { return m_Settings.autoClearBeforeRender; }
		inline RenderLayer GetRenderLayer() const { return m_Settings.renderLayer; }
		inline RenderType GetRenderType() const { return m_Settings.renderType; }
		inline int GetSortingOrder() const { return m_Settings.sortingOrder; }
		virtual void FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams) = 0;
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
		DefaultTransparentRenderContext(RenderContextCreateInfo& createInfo) : DefaultRenderContext(createInfo) {}
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

