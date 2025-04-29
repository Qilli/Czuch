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
	};
}
