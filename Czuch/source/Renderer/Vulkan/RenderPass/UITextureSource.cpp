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
		auto tex = m_Node->GetFirstColorAttachment(m_FrameGraph);

		if (!HANDLE_IS_VALID(tex))
		{
			tex = m_Node->GetDepthAttachment(m_FrameGraph);
		}

		if (!HANDLE_IS_VALID(tex))
		{
			LOG_BE_ERROR("No valid texture source found in node {0}", m_Node->name);
			return;
		}

		m_Texture = tex;

		auto texture = m_Device->AccessTexture(tex);
		auto vulkanTexture = Internal_to_Texture(texture);
		m_Descriptor = ImGui_ImplVulkan_AddTexture(vulkanTexture->sampler, vulkanTexture->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void UITextureSource::TransitionToShaderReadOnly(CommandBuffer* cmd)
	{
		if (m_Descriptor == VK_NULL_HANDLE)
		{
			Init();
		}
		m_Device->TryTransitionImageLayout(cmd, m_Texture, ImageLayout::SHADER_READ_ONLY_OPTIMAL, 0, 1);
	}

	void UITextureSource::Release()
	{
		if (m_Descriptor != VK_NULL_HANDLE)
		{
			ImGui_ImplVulkan_RemoveTexture(m_Descriptor);
			m_Descriptor = VK_NULL_HANDLE;
		}
	}

	void* UITextureSource::GetTargetTextureDescriptor()
	{
		return (void*)m_Descriptor;
	}

	U32 UITextureSource::GetTextureGlobalIndex() const
	{
		return m_Texture.globalIndex;
	}

}

