#pragma once
#include"Renderer/RenderPassControl.h"
#include"Renderer/Vulkan/VulkanCore.h"
#include"Renderer/Vulkan/VulkanDevice.h"

namespace Czuch
{
	struct FrameGraphNode;
	struct FrameGraph;
	struct UITextureSource
	{
		UITextureSource(VulkanDevice* device, FrameGraph* fgraph, FrameGraphNode* node);
		void Init();
		void* GetTargetTextureDescriptor() { return (void*)m_Descriptor; }
		FrameGraphNode* m_Node;
		FrameGraph* m_FrameGraph;
		VkDescriptorSet m_Descriptor;
		VulkanDevice* m_Device;
	};
}