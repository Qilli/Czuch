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
	class VulkanFullScreenRenderPass;
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
	public:
		VulkanRenderer(Window* window);
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
		void DrawScene(VulkanCommandBuffer* cmdBuffer, Camera* camera,RenderContextFillParams* params);
		void DrawDebugData(VulkanCommandBuffer* cmdBuffer, Camera* camera, RenderContextFillParams* params);
		void DrawFullScreenQuad(VulkanCommandBuffer* cmdBuffer, MaterialInstanceHandle material);
		void* GetRenderPassResult(Camera* cam,RenderPassType type) override;
		bool HasRenderPass(Camera* cam,RenderPassType type) override;
	public:
		void RegisterRenderPassResizeEventResponse(U32 width, U32 height, bool handleAll,std::function<void(U32,U32)>* onResize) override;
	private:
		void CreateSyncObjects();
		void ReleaseSyncObjects();
		void SubmitCommandBuffer();
		void BeforeFrameGraphExecute();
		void AfterFrameGraphExecute();

		void InitImmediateSubmitData();
	protected:
		void OnWindowResize(uint32_t width, uint32_t height) override;
		void CheckForResizeQueries();
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
		void OnPostRenderUpdateContexts(Camera* cam,RenderContextFillParams* fillParams) override;
		void OnPreRenderUpdateDebugDrawElements(Camera* cam, RenderContextFillParams* fillParams) override;
	public:
		void* GetFrameGraphFinalResult(Camera* cam) override;
	private: //frame graph control
		void CreateFrameGraphs();
	private:
		VulkanMainRenderPass* m_FinalRenderPass;
		VulkanFullScreenRenderPass* m_FullScreenRenderPass;
		std::vector<RenderPassResizeQuery> m_RenderPassResizeQueries;
		FrameData m_FramesData[MAX_FRAMES_IN_FLIGHT];
		ImmediateSubmitData m_ImmediateSubmitData;
		Window* m_AttachedWindow;
		VulkanDevice* m_Device;
		uint32_t m_CurrentFrame = 0;
		RenderContextFillParams m_DefaultContextFillParams;
		Scene* m_ActiveScene;
		int m_LastWidth = 0;
		int m_LastHeight = 0;
	};

}

