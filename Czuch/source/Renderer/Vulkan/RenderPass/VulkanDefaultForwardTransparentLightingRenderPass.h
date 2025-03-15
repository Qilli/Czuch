#pragma once
#include"VulkanRenderPassControlBase.h"

namespace Czuch
{
	class VulkanRenderer;
	class VulkanDevice;
	class VulkanDefaultForwardTransparentLightingRenderPass : public VulkanRenderPassControlBase
	{
	public:
		VulkanDefaultForwardTransparentLightingRenderPass(VulkanRenderer* renderer, VulkanDevice* device, U32 width, U32 height, bool handleWindowResize);
		virtual ~VulkanDefaultForwardTransparentLightingRenderPass() = default;
	protected:
		void CreateRenderContexts() override;
	};
}
