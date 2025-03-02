#pragma once
#include"VulkanRenderPassControlBase.h"

namespace Czuch
{
	class VulkanRenderer;
	class VulkanDevice;
	class VulkanDepthPrepassRenderPass : public VulkanRenderPassControlBase
	{
	public:
		VulkanDepthPrepassRenderPass(VulkanRenderer* renderer, VulkanDevice* device,U32 width, U32 height, bool handleWindowResize);
		virtual ~VulkanDepthPrepassRenderPass() = default;
		void* GetRenderPassResult() override;
		void Init() override;
	};
}
