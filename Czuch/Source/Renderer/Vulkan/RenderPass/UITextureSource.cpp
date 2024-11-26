#include "czpch.h"
#include "UITextureSource.h"
#include"Renderer/FrameGraph/FrameGraph.h"
#include"backends/imgui_impl_vulkan.h"
namespace Czuch
{
	UITextureSource::UITextureSource(VulkanDevice* device, FrameGraph* fgraph, FrameGraphNode* node) :m_Device(device), m_FrameGraph(fgraph), m_Node(node), m_Descriptor(VK_NULL_HANDLE)
	{
	}
	void UITextureSource::Init()
	{
		if (m_Descriptor != VK_NULL_HANDLE)
		{
			return;
		}
		//get first color texture from selected node
		auto colorTexture = m_Node->GetFirstColorAttachment(m_FrameGraph);
		auto texture = m_Device->AccessTexture(colorTexture);
		auto vulkanTexture = Internal_to_Texture(texture);
		m_Descriptor = ImGui_ImplVulkan_AddTexture(vulkanTexture->sampler, vulkanTexture->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void UITextureSource::Release()
	{
		if (m_Descriptor != VK_NULL_HANDLE)
		{
			ImGui_ImplVulkan_RemoveTexture(m_Descriptor);
			m_Descriptor = VK_NULL_HANDLE;
		}
	}

}

