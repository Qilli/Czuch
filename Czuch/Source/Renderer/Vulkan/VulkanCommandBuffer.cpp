#include "czpch.h"
#include "VulkanCommandBuffer.h"
#include"VulkanDevice.h"
#include"VulkanCore.h"
#include"DescriptorAllocator.h"

namespace Czuch
{
	const CzuchStr Tag = "[VulkanCommandBuffer]";

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer cmdBuffer):m_Device(nullptr),m_Cmd(cmdBuffer),m_isRecording(false)
	{
		m_ClearValueColor.color = { 0,0,0,1 };
		m_ClearValueColor.depthStencil = { 1.0f,255 };
		m_CurrentFrameBuffer = INVALID_HANDLE(FrameBufferHandle);
		m_CurrentRenderPass = INVALID_HANDLE(RenderPassHandle);
		m_CurrentPipeline = INVALID_HANDLE(PipelineHandle);
	}

	void VulkanCommandBuffer::Init(const GraphicsDevice* gpu)
	{
		m_Device = (VulkanDevice*)gpu;
	}

	void VulkanCommandBuffer::Release()
	{
	}

	void VulkanCommandBuffer::Begin(CommandBufferUseFlag flags)
	{
		if (m_isRecording == false)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = ConvertCommandBufferUsageFlags(flags); // Optional
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
		if (HANDLE_IS_VALID(m_CurrentRenderPass) && m_isRecording)
		{
			vkCmdEndRenderPass(m_Cmd);
			m_CurrentRenderPass = INVALID_HANDLE(RenderPassHandle);
		}
	}

	void VulkanCommandBuffer::DrawMesh(const RenderObjectInstance& renderElement, DescriptorAllocator* allocator)
	{
		CZUCH_BE_ASSERT(renderElement.IsValid(), "Render object instance passed to command buffer is invalid");

		Mesh* meshInstance = m_Device->AccessMesh(renderElement.mesh);
		if (meshInstance != nullptr)
		{
			MaterialInstance* materialInstance = m_Device->AccessMaterialInstance(HANDLE_IS_VALID(renderElement.overrideMaterial)? renderElement.overrideMaterial:meshInstance->materialHandle);
			Material* material = m_Device->AccessMaterial(materialInstance->handle);
			auto& paramsDesc = materialInstance->params.shaderParamsDesc;
			BindPipeline(material->pipeline);

			auto pipelinePtr = m_Device->AccessPipeline(material->pipeline);
			DescriptorWriter writer;

			for (int a = 0; a < material->desc.pipelineDesc.layoutsCount; a++)
			{
				writer.Clear();
				auto descriptorLayout = pipelinePtr->layouts[a];
				auto layout = m_Device->AccessDescriptorSetLayout(descriptorLayout);
				auto descriptor = allocator->Allocate(paramsDesc[a],layout);

				for (int b = 0; b < layout->desc.bindingsCount; b++)
				{
					auto binding = layout->desc.bindings[b];
					if (binding.type == DescriptorType::UNIFORM_BUFFER)
					{
						writer.WriteBuffer(binding.index, m_Device->AccessBuffer(BufferHandle(paramsDesc[a].descriptors[b].resource)), binding.size, 0, binding.type);
					}
					else if (binding.type == DescriptorType::SAMPLER)
					{
						writer.WriteTexture(binding.index, m_Device->AccessTexture(TextureHandle(paramsDesc[a].descriptors[b].resource)), DescriptorType::SAMPLER);
					}
					writer.UpdateSet(m_Device, descriptor);
				}

				BindDescriptorSet(descriptor, a, 1, nullptr, 0);
			}

			BindVertexBuffer(meshInstance->positionsHandle, 0, 0);
			if (meshInstance->HasColors())
			{
				BindVertexBuffer(meshInstance->colorsHandle, 1, 0);
			}

			if (meshInstance->HasUV0())
			{
				BindVertexBuffer(meshInstance->uvs0Handle, 2, 0);
			}

			if (meshInstance->HasNormals())
			{
				BindVertexBuffer(meshInstance->normalsHandle, 3, 0);
			}

			auto vulkanPipeline= Internal_To_Pipeline(pipelinePtr);
			vkCmdPushConstants(m_Cmd, vulkanPipeline->pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4x4), (void*) & renderElement.localToClipSpaceTransformation);

			BindIndexBuffer(meshInstance->indicesHandle, 0);
			DrawIndexed(m_Device->AccessBuffer(meshInstance->indicesHandle)->desc.elementsCount);
		}
	}

	void VulkanCommandBuffer::BindPass(RenderPassHandle renderpass, FrameBufferHandle framebuffer)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(renderpass), "render pass passed to command buffer bind pass is null");
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(framebuffer), "framebuffer passed to command buffer bind pass is null");

		if (renderpass.handle == m_CurrentRenderPass.handle && m_CurrentFrameBuffer.handle == framebuffer.handle)
		{
			return;
		}

		m_CurrentFrameBuffer = framebuffer;
		m_CurrentRenderPass = renderpass;

		auto fb = Internal_to_Framebuffer(m_Device->AccessFrameBuffer(framebuffer));
		VkExtent2D extent{};
		extent.width = fb->createInfo.width;
		extent.height = fb->createInfo.height;

		VkClearValue clearValues[] = { m_ClearValueColor,m_ClearValueDepth };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = Internal_to_RenderPass(m_Device->AccessRenderPass(renderpass))->renderPass;
		renderPassInfo.framebuffer = fb->framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(m_Cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanCommandBuffer::BindPipeline(PipelineHandle pipeline)
	{
		if (!HANDLE_IS_VALID(pipeline))
		{
			return;
		}
		vkCmdBindPipeline(m_Cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Internal_To_Pipeline(m_Device->AccessPipeline(pipeline))->pipeline);
		m_CurrentPipeline = pipeline;
	}

	void VulkanCommandBuffer::BindVertexBuffer(BufferHandle buffer, U32 binding=0, U32 offset=0)
	{
		VkBuffer vertexBuffers[] = { Internal_to_Buffer(m_Device->AccessBuffer(buffer))->buffer };
		VkDeviceSize offsets[] = { offset };
		vkCmdBindVertexBuffers(m_Cmd, binding, 1, vertexBuffers, offsets);
	}

	void VulkanCommandBuffer::BindIndexBuffer(BufferHandle buffer, U32 offset=0)
	{
		vkCmdBindIndexBuffer(m_Cmd, Internal_to_Buffer(m_Device->AccessBuffer(buffer))->buffer, offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanCommandBuffer::BindDescriptorSet(DescriptorSet* descriptor,U16 setIndex, U32 num, U32* offsets, U32 num_offsets)
	{
		auto pp = m_Device->AccessPipeline(m_CurrentPipeline);
		if (pp == nullptr)
		{
			LOG_BE_ERROR("{0} Failed to bind descriptor set, pipeline is invalid", Tag);
			return;
		}
		auto vulkanPipeline = Internal_To_Pipeline(pp);
		vkCmdBindDescriptorSets(m_Cmd, ConvertBindPoint(pp->m_desc.bindPoint), vulkanPipeline->pipelineLayout, setIndex, num,&descriptor->descriptorSet,num_offsets,offsets);
	}

	void VulkanCommandBuffer::SetClearColor(float r, float g, float b, float a)
	{
		m_ClearValueColor.color = VkClearColorValue{r,g,b,a};
	}

	void VulkanCommandBuffer::SetDepthStencil(float depth, U8 stencil)
	{
		m_ClearValueDepth.depthStencil = { depth,stencil };
	}

	void VulkanCommandBuffer::SetViewport(ViewportDesc viewport)
	{
		VkViewport viewportVk{};
		viewportVk.x = 0.0f;
		viewportVk.y = viewport.height;
		viewportVk.width = viewport.width;
		viewportVk.height = -viewport.height;
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

	void VulkanCommandBuffer::BeginDynamicRenderPass(VkImageView colorView, VkImageView depthView, U32 width, U32 height)
	{
		VkRenderingAttachmentInfoKHR colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachment.imageView = colorView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue.color = { 0.0f,0.0f,0.0f,0.0f };

		VkRenderingAttachmentInfoKHR depthStencilAttachment{};
		depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthStencilAttachment.imageView = depthView;
		depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthStencilAttachment.clearValue.depthStencil = { 1.0f,  0 };

		auto render_area = VkRect2D{ VkOffset2D{}, VkExtent2D{width, height} };
		VkRenderingInfoKHR  render_info = {};
		render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		render_info.layerCount = 1;
		render_info.pDepthAttachment = &depthStencilAttachment;
		render_info.pStencilAttachment = &depthStencilAttachment;
		render_info.pColorAttachments = &colorAttachment;
		render_info.renderArea = render_area;
		render_info.colorAttachmentCount = 1;

		vkCmdBeginRendering(m_Cmd, &render_info);
	}

	void VulkanCommandBuffer::EndDynamicRenderPass()
	{
		vkCmdEndRendering(m_Cmd);
	}

}
