#pragma once
#include"Renderer/Renderer.h"
#include"VulkanBase.h"

namespace Czuch
{
	class VulkanDevice;
	class VulkanCommandBuffer;
	class Window;
	class DescriptorAllocator;
	struct DescriptorSet;

	class CZUCH_API VulkanRenderer final: public Renderer
	{
	public:
		static RendererAPI GetUsedAPI() { return RendererAPI::Vulkan; }
		static const int MAX_FRAMES_IN_FLIGHT = 2;
	public:
		VulkanRenderer(Window* window,ValidationMode validationMode = ValidationMode::Disabled);
		~VulkanRenderer() override;
		
		void Init() override;
		void DrawFrame() override;
		void AwaitDeviceIdle() override;
		GraphicsDevice* GetDevice() override;
	private:
		void CreateSyncObjects();
		void ReleaseSyncObjects();
		void RecordCommandBuffer(uint32_t imageIndex);
		void SubmitCommandBuffer();

		void InitSceneData();
		void SetSceneData();

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

		struct SceneDataContainer
		{
			SceneData data;
			BufferDesc bufferDesc;
			DescriptorSet* descriptor;
			DescriptorSet* descriptorTex;
			TextureHandle tex;
			BufferHandle buffer[MAX_FRAMES_IN_FLIGHT];
			AssetHandle materialHandle;
		};

	private:
		inline FrameData& GetCurrentFrame() {
			return m_FramesData[m_CurrentFrame];
		}

	private:

		FrameData m_FramesData[MAX_FRAMES_IN_FLIGHT];
		ValidationMode m_RendererValidationMode;
		Window* m_AttachedWindow;
		VulkanDevice* m_Device;
		uint32_t m_CurrentFrame = 0;
		SceneDataContainer m_SceneData;
	};

}

