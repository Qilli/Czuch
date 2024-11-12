#pragma once
#include"VulkanRenderPassControlBase.h"

namespace Czuch
{
	class VulkanRenderer;
	class VulkanDevice;
	class VulkanDefaultForwardLightingRenderPass : public VulkanRenderPassControlBase
	{
	public:
		VulkanDefaultForwardLightingRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize);
		virtual ~VulkanDefaultForwardLightingRenderPass() = default;
	};
}
