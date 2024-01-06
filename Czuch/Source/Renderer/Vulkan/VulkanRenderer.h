#pragma once
#include"Renderer/Renderer.h"
#include"VulkanBase.h"

namespace Czuch
{
	class VulkanDevice;
	class VulkanCommandBuffer;
	class Window;

	class CZUCH_API VulkanRenderer final: public Renderer
	{
	public:
		static RendererAPI GetUsedAPI() { return RendererAPI::Vulkan; }
		const int MAX_FRAMES_IN_FLIGHT = 2;
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
	private:
		ValidationMode m_RendererValidationMode;
		Window* m_AttachedWindow;
		VulkanDevice* m_Device;
		std::vector<VulkanCommandBuffer*> m_CmdBuffers;
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		uint32_t m_CurrentFrame = 0;
	};

}

