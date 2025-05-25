#pragma once
#include"VulkanRenderPassControlBase.h"

namespace Czuch
{
	class VulkanRenderer;
	class VulkanDevice;
	class VulkanDebugDrawRenderPass : public VulkanRenderPassControlBase
	{
	public:
		VulkanDebugDrawRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize);
		virtual ~VulkanDebugDrawRenderPass() = default;
		void Execute(CommandBuffer* cmdBuffer) override;
		void BeforeFrameGraphExecute(CommandBuffer* cmd, Renderer* renderer) override;
		void Init() override;
	private:
		RenderContextFillParams m_DebugFillParams[3];
	};
}
