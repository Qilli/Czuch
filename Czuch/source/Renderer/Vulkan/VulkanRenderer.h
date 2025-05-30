#pragma once
#include"Renderer/Renderer.h"
#include"VulkanBase.h"
#include"Renderer/RenderContext.h"
#include"Renderer/RenderPassControl.h"
#include"Renderer/FrameGraph/FrameGraphBuilderHelper.h"
#include<functional>

namespace Czuch
{
	class VulkanMainRenderPass;
	class VulkanDevice;
	class VulkanCommandBuffer;
	class Window;
	class DescriptorAllocator;
	class CommandBuffer;
	struct DescriptorSet;
	class Camera;


	class CZUCH_API VulkanRenderer final : public Renderer
	{
	public:
		static RendererAPI GetUsedAPI() { return RendererAPI::Vulkan; }
		static const int MAX_FRAMES_IN_FLIGHT = 2;
	public:
		VulkanRenderer(Window* window, RenderSettings* renderSettings);
		~VulkanRenderer() override;

		void Init() override;
		void AfterSystemInit() override;
		void CreateRenderGraph() override;
		void DrawFrame() override;
		void AwaitDeviceIdle() override;
		void ReleaseDependencies() override;
		GraphicsDevice* GetDevice() override;
	public:
		void SetActiveScene(Scene* scene) override;
	public:
		void ImmediateSubmitWithCommandBuffer(std::function<void(CommandBuffer* cmd)>&& processor);
		void DrawScene(VulkanCommandBuffer* cmdBuffer,RenderContext* context);
		void DrawFullScreenQuad(VulkanCommandBuffer* cmdBuffer, MaterialInstanceHandle material);
		void* GetRenderPassResult(RenderPassType type) override;
		bool HasRenderPass(RenderPassType type) override;
	public:
		void RegisterRenderPassResizeEventResponse(U32 width, U32 height, bool handleAll,std::function<void(U32,U32)>* onResize) override;
		inline FrameGraph* GetFrameGraph() override { return &m_CurrentFrameGraph; }
	private:
		void CreateSyncObjects();
		void ReleaseSyncObjects();
		void SubmitCommandBuffer();

		void InitSceneData();
		void SetSceneData();

		void InitImmediateSubmitData();
	protected:
		void OnWindowResize(uint32_t width, uint32_t height) override;
		void CheckForResizeQueries();
		RenderPassControl* GetRenderPassByType(RenderPassType type);
	private:

		struct FrameData
		{
			DescriptorAllocator* descriptorAllocator;
			CommandBufferHandle commandBuffer;
			VkSemaphore imageAvailableSemaphore;
			VkSemaphore renderFinishedSemaphote;
			VkFence inFlightFence;
			DeletionQueue frameDeletionQueue;

			void Reset();
		};

		struct ImmediateSubmitData
		{
			CommandBufferHandle commandBuffer;
			VkFence fence;
			VkCommandPool pool;

			void Release(VulkanDevice* device);
		};

		struct TilesDataContainer
		{
			glm::ivec4 screenSize;
			Array<LightsTileData> tilesData;
		};

		struct SceneDataContainer
		{
			SceneData data;
			BufferDesc bufferDesc;
			BufferDesc lightsBufferDesc;
			BufferDesc tilesBufferDesc;
			BufferDesc lightsListBufferDesc;
			BufferHandle buffer[MAX_FRAMES_IN_FLIGHT];
			BufferHandle lightsBuffer[MAX_FRAMES_IN_FLIGHT];
			BufferHandle tilesBuffer[MAX_FRAMES_IN_FLIGHT];
			BufferHandle lightsListBuffer[MAX_FRAMES_IN_FLIGHT];

			U32 tiles_in_width;
			U32 tiles_in_height;
			U32 tiles_count;
			U32 lastLightsCount;

			Array<U32> lightsIndexList;;
			Array<LightData> lightsData;
			TilesDataContainer tilesDataContainer;

			void Init(VulkanDevice* device);
			void Release(VulkanDevice* device);
			void InitTilesBuffer(VulkanDevice* device,bool resize,U32 width,U32 height);
			bool FillTilesWithLights(VulkanDevice* device, const Array<LightObjectInfo>& allLight,U32 frame);
			void UpdateMaterialsLightsInfo();

			SceneDataBuffers GetSceneDataBuffers(U32 frame);
		};

		struct RenderPassResizeQuery
		{
			RenderPassType type;
			bool allNotHandledByWindowSizeChanged;
			U32 width;
			U32 height;
		};

	private:
		inline FrameData& GetCurrentFrame() {
			return m_FramesData[m_CurrentFrame];
		}

	public:
		void OnPreRenderUpdateContexts(Camera* cam, int width, int height,RenderContextFillParams* fillParams, RenderContext* mainRenderContext) override;
		void OnPostRenderUpdateContexts(RenderContextFillParams* fillParams,RenderContext* mainRenderContext) override;
	public:
		//render pass contorl helpers
		RenderPassControl* RegisterRenderPassControl(RenderPassControl* control) override;
		void UnRegisterRenderPassControl(RenderPassControl* control) override;
		RenderPassHandle GetNativeRenderPassHandle(RenderPassType type) override;
		void* GetFrameGraphFinalResult() override;
	private: //frame graph control
		void CreateFrameGraphs();
		void ReleaseFrameGraphs();
		FrameGraphBuilderHelper m_FrameGraphBuilder;
		FrameGraph m_CurrentFrameGraph;
	private:
		Array<RenderPassControl*> m_RenderPassControls;
		VulkanMainRenderPass* m_FinalRenderPass;
		std::vector<RenderPassResizeQuery> m_RenderPassResizeQueries;
		FrameData m_FramesData[MAX_FRAMES_IN_FLIGHT];
		ImmediateSubmitData m_ImmediateSubmitData;
		Window* m_AttachedWindow;
		VulkanDevice* m_Device;
		uint32_t m_CurrentFrame = 0;
		SceneDataContainer m_SceneData;
		RenderContextFillParams m_DefaultContextFillParams;
		Scene* m_ActiveScene;
		int m_LastWidth = 0;
		int m_LastHeight = 0;
	};

}

