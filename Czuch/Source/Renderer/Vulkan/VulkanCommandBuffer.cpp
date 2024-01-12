#include "czpch.h"
#include "VulkanCommandBuffer.h"
#include"VulkanCore.h"

namespace Czuch
{
	const CzuchStr Tag = "[VulkanCommandBuffer]";

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer cmdBuffer):m_Device(nullptr),m_Cmd(cmdBuffer),m_isRecording(false)
	{
		m_ClearValue.color = { 0,0,0,1 };
		m_ClearValue.depthStencil = { 1.0f,255 };
		m_CurrentFrameBuffer = nullptr;
		m_CurrentRenderPass = nullptr;
		m_CurrentPipeline = nullptr;
	}

	void VulkanCommandBuffer::Init(const GraphicsDevice* gpu)
	{
		m_Device = (VulkanDevice*)gpu;
	}

	void VulkanCommandBuffer::Release()
	{
	}

	void VulkanCommandBuffer::Begin()
	{
		if (m_isRecording == false)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional

			vkResetCommandBuffer(m_Cmd, 0);

			if (vkBeginCommandBuffer(m_Cmd, &beginInfo) != VK_SUCCESS) {
				LOG_BE_ERROR("{0} Failed to begin recording of command buffer.", Tag);
				return;
			}
			m_isRecording = true;
		}
	}

	void VulkanCommandBuffer::End()
	{
		if (m_isRecording)
		{
			vkEndCommandBuffer(m_Cmd);
			m_isRecording = false;
		}
	}

	void VulkanCommandBuffer::EndCurrentRenderPass()
	{
		if (m_CurrentRenderPass != nullptr && m_isRecording)
		{
			vkCmdEndRenderPass(m_Cmd);
			m_CurrentRenderPass = nullptr;
		}
	}

	void VulkanCommandBuffer::BindPass(RenderPass* renderpass, FrameBuffer* framebuffer)
	{
		CZUCH_BE_ASSERT(renderpass != nullptr, "render pass passed to command buffer bind pass is null");
		CZUCH_BE_ASSERT(framebuffer!=nullptr, "framebuffer passed to command buffer bind pass is null");

		if (renderpass == m_CurrentRenderPass && m_CurrentFrameBuffer == framebuffer)
		{
			return;
		}

		m_CurrentFrameBuffer = framebuffer;
		m_CurrentRenderPass = renderpass;

		auto fb = Internal_to_Framebuffer(framebuffer);
		VkExtent2D extent{};
		extent.width = fb->createInfo.width;
		extent.height = fb->createInfo.height;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = Internal_to_RenderPass(renderpass)->renderPass;
		renderPassInfo.framebuffer = fb->framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &m_ClearValue;

		vkCmdBeginRenderPass(m_Cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanCommandBuffer::BindPipeline(Pipeline* pipeline)
	{
		if (pipeline == nullptr)
		{
			return;
		}
		vkCmdBindPipeline(m_Cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Internal_To_Pipeline(pipeline)->pipeline);
		m_CurrentPipeline = pipeline;
	}

	void VulkanCommandBuffer::BindVertexBuffer(Buffer* buffer, U32 binding=0, U32 offset=0)
	{
		VkBuffer vertexBuffers[] = { Internal_to_Buffer(buffer)->buffer };
		VkDeviceSize offsets[] = { offset };
		vkCmdBindVertexBuffers(m_Cmd, binding, 1, vertexBuffers, offsets);
	}

	void VulkanCommandBuffer::BindIndexBuffer(Buffer* buffer, U32 offset=0)
	{
		vkCmdBindIndexBuffer(m_Cmd, Internal_to_Buffer(buffer)->buffer, offset, VK_INDEX_TYPE_UINT16);
	}

	void VulkanCommandBuffer::BindDescriptorSet(DescriptorSet* descriptor, U32 num, U32* offsets, U32 num_offsets)
	{
	}

	void VulkanCommandBuffer::SetClearColor(float r, float g, float b, float a)
	{
		m_ClearValue.color = VkClearColorValue{r,g,b,a};
	}

	void VulkanCommandBuffer::SetDepthStencil(float depth, U8 stencil)
	{
		m_ClearValue.depthStencil = { depth,stencil };
	}

	void VulkanCommandBuffer::SetViewport(ViewportDesc viewport)
	{
		VkViewport viewportVk{};
		viewportVk.x = 0.0f;
		viewportVk.y = 0.0f;
		viewportVk.width = viewport.width;
		viewportVk.height = viewport.height;
		viewportVk.minDepth = viewport.minDepth;
		viewportVk.maxDepth = viewport.maxDepth;
		vkCmdSetViewport(m_Cmd, 0, 1, &viewportVk);
	}

	void VulkanCommandBuffer::SetScrissors(ScissorsDesc scissors)
	{
		VkRect2D scissorVk{};
		scissorVk.offset = { scissors.offsetX, scissors.offsetY };
		scissorVk.extent.width=scissors.width;
		scissorVk.extent.height = scissors.height;
		vkCmdSetScissor(m_Cmd, 0, 1, &scissorVk);
	}

	void VulkanCommandBuffer::Draw(U32 vertexCount, U32 firstVertex, U32 instanceCount, U32 firstInstance)
	{
		vkCmdDraw(m_Cmd, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexed(U32 indicesCount, U32 firstIndex, U32 instanceCount, U32 firstnstance,U32 vertexOffset)
	{
		vkCmdDrawIndexed(m_Cmd, indicesCount, instanceCount, firstIndex, vertexOffset, firstnstance);
	}

}
