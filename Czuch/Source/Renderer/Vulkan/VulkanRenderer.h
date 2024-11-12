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
		void DrawFrame() override;
		void AwaitDeviceIdle() override;
		GraphicsDevice* GetDevice() override;
	public:
		bool RegisterRenderContext(RenderContext* context) override;
		void UnRegisterRenderContext(RenderContext* context) override;
		void SetActiveScene(Scene* scene) override;
	public:
		void ImmediateSubmitWithCommandBuffer(std::function<void(CommandBuffer* cmd)>&& processor);
		void DrawScene(VulkanCommandBuffer* cmdBuffer);
		void* GetRenderPassResult(RenderPassType type) override;
		bool HasRenderPass(RenderPassType type) override;
	public:
		void RegisterRenderPassResizeEventResponse(U32 width, U32 height, bool handleWindowResize,std::function<void(U32,U32)>* onResize) override;
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

		struct SceneDataContainer
		{
			SceneData data;
			BufferDesc bufferDesc;
			BufferHandle buffer[MAX_FRAMES_IN_FLIGHT];
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
		void OnPreRenderUpdateContexts(Camera* cam, int width, int height,RenderContextFillParams* fillParams) override;
		void OnPostRenderUpdateContexts(RenderContextFillParams* fillParams) override;
	public:
		//render pass contorl helpers
		RenderPassControl* RegisterRenderPassControl(RenderPassControl* control) override;
		void UnRegisterRenderPassControl(RenderPassControl* control) override;
		RenderPassHandle GetNativeRenderPassHandle(RenderPassType type) override;
	private: //frame graph control
		void CreateFrameGraphs();
		void ReleaseFrameGraphs();
		FrameGraphBuilderHelper m_FrameGraphBuilder;
		FrameGraph m_CurrentFrameGraph;
	private:
		Array<RenderPassControl*> m_RenderPassControls;
		VulkanMainRenderPass* m_FinalRenderPass;
		std::vector<RenderPassResizeQuery> m_RenderPassResizeQueries;
		RenderContextContainer m_MainRenderContexts;
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

