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
		void TransitionToShaderReadOnly(CommandBuffer* cmd);
		void Release();
		void* GetTargetTextureDescriptor();
		FrameGraphNode* m_Node;
		FrameGraph* m_FrameGraph;
		VkDescriptorSet m_Descriptor;
		TextureHandle m_Texture;
		VulkanDevice* m_Device;
	};
}