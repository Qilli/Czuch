#pragma once
#include"VulkanRenderPassControlBase.h"

namespace Czuch
{
	class VulkanRenderer;
	class VulkanDevice;
	class VulkanDepthLinearPrepassRenderPass : public VulkanRenderPassControlBase
	{
	public:
		VulkanDepthLinearPrepassRenderPass(VulkanRenderer* renderer, VulkanDevice* device,U32 width, U32 height, bool handleWindowResize);
		virtual ~VulkanDepthLinearPrepassRenderPass() = default;
		void Execute(CommandBuffer* cmd) override;
		void PreDraw(CommandBuffer* cmdBuffer, Renderer* renderer) override;
		void Init() override;
		void Release() override;
	private:
		bool m_MaterialsAdded = false;
		BufferHandle m_PlanesBuffer;
		CameraPlanesData m_PlanesData;
		StringID m_PlanesBufferID;
	};
}
