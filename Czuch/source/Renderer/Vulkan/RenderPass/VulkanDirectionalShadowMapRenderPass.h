#pragma once
#include"VulkanRenderPassControlBase.h"

namespace Czuch
{
	class VulkanRenderer;
	class VulkanDevice;
	class VulkanDirectionalShadowMapRenderPass : public VulkanRenderPassControlBase
	{
	public:
		VulkanDirectionalShadowMapRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize);
		virtual ~VulkanDirectionalShadowMapRenderPass() = default;
		void Init() override;
	};
}
