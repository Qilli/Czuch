#pragma once
#include "Graphics.h"
#include "Core/Math.h"
#include "DebugDraw.h"
#include "RendererCommon.h"
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

    struct SceneCameraRenderingControl
	{
		Camera *camera;
		SceneData sceneData;
		BufferDesc sceneDataBufferDesc;
		BufferDesc lightsBufferDesc;
		BufferDesc tilesBufferDesc;
		BufferDesc lightsListBufferDesc;
		BufferDesc renderObjectsBufferDesc;
		BufferHandle sceneDataBuffer[MAX_FRAMES_IN_FLIGHT];
		BufferHandle lightsBuffer[MAX_FRAMES_IN_FLIGHT];
		BufferHandle tilesBuffer[MAX_FRAMES_IN_FLIGHT];
		BufferHandle lightsListBuffer[MAX_FRAMES_IN_FLIGHT];
		BufferHandle renderObjectsBuffer[MAX_FRAMES_IN_FLIGHT];

		U32 tiles_in_width;
		U32 tiles_in_height;
		U32 tiles_count;
		U32 lastLightsCount;

		Array<U32> lightsIndexList;
		Array<LightData> lightsData;
		Array<RenderObjectGPUData> renderObjectsData;
		TilesDataContainer tilesDataContainer;
		bool lightsChanged = true;

		void Init(GraphicsDevice *device, Camera *cam);
		void OnSceneActive(GraphicsDevice *device);
		void Release(GraphicsDevice *device);
		void InitTilesBuffer(GraphicsDevice *device, bool resize, U32 width, U32 height);
		bool FillTilesWithLights(GraphicsDevice *device, const Array<LightObjectInfo> &allLight, U32 frame);
		/// <summary>
		/// Fill buffer with render objects data(materials info, transforms). If returns true thats mean we need to update materials for new buffer size
		/// </summary>
		/// <param name="device"></param>
		/// <param name="allObjects"></param>
		/// <param name="frame"></param>
		bool FillRenderObjectsData(GraphicsDevice *device, RenderObjectsContainer &allObjects, U32 frame);
		/// <summary>
		/// Init render objects buffer, resize if needed
		/// </summary>
		/// <param name="device"></param>
		/// <param name="resize"></param>
		/// <param name="frame"></param>
		void InitRenderObjectsBuffer(GraphicsDevice *device, bool resize, U32 size);
		void UpdateMaterialsLightsInfo();
		void UpdateMaterialsRenderObjectsInfo();

		SceneDataBuffers GetSceneDataBuffers(U32 frame);
		void UpdateSceneDataBuffers(IScene *scene, GraphicsDevice *device, RenderObjectsContainer &visibleObjects, U32 frame, DeletionQueue &deletionQueue);
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
		void CreateAndInitLinesBuffer(GraphicsDevice *device);
		void ReleaseLinesBuffer(GraphicsDevice *device);

	private:
		BufferDesc m_TrianglesBufferDesc;
		BufferDesc m_VertexBufferTrianglesDesc;
		BufferDesc m_IndexBufferTrianglesDesc;
		BufferHandle m_VertexBufferTriangles;
		BufferHandle m_IndexBufferTriangles;
		BufferHandle m_TrianglesBuffer[MAX_FRAMES_IN_FLIGHT];
		IndirectDrawForCommandBufferData m_IndirectDrawDataTriangles;
		void CreateAndInitTrianglesBuffer(GraphicsDevice *device);
		void ReleaseTrianglesBuffer(GraphicsDevice *device);

	private:
		BufferDesc m_PointsBufferDesc;
		BufferDesc m_VertexBufferPointsDesc;
		BufferHandle m_VertexBufferPoints;
		BufferHandle m_PointsBuffer[MAX_FRAMES_IN_FLIGHT];
		IndirectDrawForCommandBufferData m_IndirectDrawDataPoints;
		void CreateAndInitPointsBuffer(GraphicsDevice *device);
		void ReleasePointsBuffer(GraphicsDevice *device);

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
		Camera *m_Camera;

	public:
		void UpdateSceneDataBuffer(IScene *scene, GraphicsDevice *device, U32 frame);
		void OnSceneActive(Camera *cam, GraphicsDevice *device);
		void Release(GraphicsDevice *device);
		bool FillDebugBuffersData(GraphicsDevice *device, U32 frame);
		void FillSceneDataBuffer(Czuch::GraphicsDevice *device, const Czuch::MaterialInstanceHandle debugLightsMaterial, U32 frame);
		void UpdateDebugMaterialInfo(GraphicsDevice *device, U32 frame);
		IndirectDrawForCommandBufferData &FillAndGetIndirectDrawDataForDebugLinesDrawing(MaterialInstanceHandle material, U32 frame);
		IndirectDrawForCommandBufferData &FillAndGetIndirectDrawDataForDebugTrianglesDrawing(MaterialInstanceHandle material, U32 frame);
		IndirectDrawForCommandBufferData &FillAndGetIndirectDrawDataForDebugPointsDrawing(MaterialInstanceHandle material, U32 frame);

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
		void DrawLine(const Vec3 &start, const Vec3 &end, const Color &color) override;
		void DrawTriangle(const Vec3 &a, const Vec3 &b, const Vec3 &c, const Color &color) override;
		void DrawPoint(const Vec3 &point, const Color &color, const float size) override;
		void DrawMesh(const AssetHandle mesh, const Mat4x4 &transform, const Color &color) override;
		void DrawQuad(const Vec3 &center, const Vec3 &normal, float size, const Color &color) override;
		void DrawCircle(const Vec3 &center, const Vec3 &normal, float radius, const Color &color) override;
		void DrawLinesSphere(const Vec3 &center, float radius, const Color &color) override;
		void DrawCone(const Vec3 &position, const Vec3 &direction, float range, float angle, const Color &color) override;
		void DrawLinesList(const Array<Vec3> &points, const Color &color) override;
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
		FrameGraphBuilderHelper *m_FrameGraphBuilder;
		FrameGraph *m_FrameGraph;
		Array<RenderPassControl *> m_RenderPassControls;

	public:
		void CreateFrameGraph(Camera *camera, GraphicsDevice *device, Renderer *renderer, Vec2 targetSize, bool handleWindowResize);
		void ReleaseFrameGraph();
		RenderPassControl *RegisterRenderPassControl(RenderPassControl *control);
		void UnRegisterRenderPassControl(RenderPassControl *control);
		void *GetFrameGraphFinalResult();
		RenderPassControl *GetRenderPassControlByType(RenderPassType type) const;
		bool HasRenderPass(RenderPassType type);
		void OnResize(U32 width, U32 height, bool windowSizeChanged);
		void ResizeRenderPassType(RenderPassType type, U32 width, U32 height);
		void Init();
		/// <summary>
		/// Before new frame is renderered we call this method to prepare frame graph
		/// </summary>
		/// <param name="cmdBuffer"></param>
		void BeforeFrameGraphExecute(CommandBuffer *cmdBuffer);
		void AfterFrameGraphExecute(CommandBuffer *cmdBuffer);
		void Execute(GraphicsDevice *device, CommandBuffer *cmdBuffer);
		Camera *GetCamera() const;

		FinalFrameGraphNodeInfo GetFinalFrameGraphNodeInfo() const;
		void SetDebugRenderFlag(DebugRenderingFlag flag, bool enable);
		void SetDebugRenderFlagsGroup(U32 flags);

		FrameGraph *GetFrameGraph() const { return m_FrameGraph; }
	};

	struct SceneCameraRendering
	{
		SceneCameraRenderingControl cameraControl;
		SceneCameraDebugRenderingControl debugCameraControl;
		IScene *activeScene;

		void Release(GraphicsDevice *device);
		void OnSceneActive(Camera *camera, GraphicsDevice *device, IScene *scene);
	};

	struct CameraLightsInfo
	{
		LightComponent *directionalLight = nullptr;
		LightComponent *spotAndPointLights[MAX_LIGHTS_WITH_SHADOWS];
		TransformComponent *directionalCameraShadowCasterTransform = nullptr;

		Camera directionalLightCamera;
		Camera spotAndPointLightsCamera[MAX_LIGHTS_WITH_SHADOWS];

		void Reset();
		void Rebuild(IScene *scene, FrameGraphControl *frameGraph, Camera *camera, IDebugDrawBuilder *debugDraw);

		U32 lightsCount = 0;
		bool HasDirectionalLightWithShadows() const { return directionalLight != nullptr; }
		Camera *GetShadowCastingCameraForDirectionalLight() { return &directionalLightCamera; }
	};

	struct SceneCameraControl
	{
		SceneCameraControl()
		{
			camera = nullptr;
			currentScene = nullptr;
			isPrimaryCamera = false;
		}

		Camera *camera;
		IScene *currentScene;
		SceneCameraRendering cameraRendering;
		bool isPrimaryCamera;
		Array<RenderContextControl> renderContexts;
		RenderObjectsContainer visibleRenderObjects;
		FrameGraphControl frameGraphControl;
		CameraLightsInfo cameraLightsInfo;

		Vec3 ComputeDirectionalShadowCasterPosition(); // Compute position for directional light shadow caster based on current camera position and direction

		Camera *GetCameraForContext(RenderPassType renderPassType);

		/// <summary>
		/// Release is called when we are leaving the scene or when the scene is destroyed or when camera is removed
		/// We clear all render data for current camera
		/// </summary>
		void Release(GraphicsDevice *device);
		/// <summary>
		/// This method is called when the scene is activated. Here we need to prepare the camera control for rendering
		/// </summary>
		/// <param name="renderer"></param>
		/// <param name="device"></param>
		/// <param name="scene"></param>
		void OnSceneActive(Renderer *renderer, GraphicsDevice *device, IScene *scene);
		void AfterSceneActive();
		void OnResize(GraphicsDevice *device, U32 width, U32 height, bool windowSizeChanged);
		SceneDataBuffers GetSceneDataBuffers(U32 frame);
		void UpdateSceneDataBuffers(GraphicsDevice *device, U32 frame, DeletionQueue &deletionQueue);
		void UpdateLightsInfo();

		RenderContext *GetRenderContext(RenderPassType type, bool createIfNotExist = true);
		void AddRenderContext(RenderContextCreateInfo ctx, RenderPassType type);
		bool IsPrimaryCamera() const { return isPrimaryCamera; }
		RenderContext *FillRenderList(GraphicsDevice *device, RenderContextFillParams &fillParams);
		void FillDebugDrawElements(GraphicsDevice *device, RenderContextFillParams &fillParams);
		IndirectDrawForCommandBufferData &GetIndirectDrawDataForDebugDrawingLines(RenderContextFillParams &fillParams, U32 frame);
		IndirectDrawForCommandBufferData &GetIndirectDrawDataForDebugDrawingTriangles(RenderContextFillParams &fillParams, U32 frame);
		IndirectDrawForCommandBufferData &GetIndirectDrawDataForDebugDrawingPoints(RenderContextFillParams &fillParams, U32 frame);
		/// <summary>
		/// make contexts dirty so we can update them at the beginning of the next frame
		/// </summary>
		void OnPostRender();
		void UpdateVisibleObjects(RenderObjectsContainer &allObjects);

	private:
		void ReleaseRenderContexts();
	};
}