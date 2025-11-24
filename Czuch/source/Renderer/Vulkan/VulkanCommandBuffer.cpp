#include "czpch.h"
#include "VulkanCommandBuffer.h"
#include"VulkanDevice.h"
#include"VulkanCore.h"
#include"DescriptorAllocator.h"

namespace Czuch
{
	const CzuchStr Tag = "[VulkanCommandBuffer]";

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer cmdBuffer) :m_Device(nullptr), m_Cmd(cmdBuffer), m_isRecording(false)
	{
		m_ClearValueColor.color = { 0,0,0,1 };
		m_ClearValueColor.depthStencil = { 1.0f,255 };
		m_CurrentFrameBuffer = INVALID_HANDLE(FrameBufferHandle);
		m_CurrentRenderPass = INVALID_HANDLE(RenderPassHandle);
		m_CurrentPipeline = INVALID_HANDLE(PipelineHandle);
		m_CurrentMaterialHandle = INVALID_HANDLE(MaterialInstanceHandle);
	}

	void VulkanCommandBuffer::Init(const GraphicsDevice* gpu)
	{
		m_Device = (VulkanDevice*)gpu;
		m_ColorAttachmentsInfo.resize(8);
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

			if (vkResetCommandBuffer(m_Cmd, 0) != VK_SUCCESS)
			{
				LOG_BE_ERROR("{0} Failed to reset command buffer.", Tag);
				return;
			}

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

			INVALIDATE_HANDLE(m_CurrentVertexBuffer);
			INVALIDATE_HANDLE(m_CurrentIndexBuffer);
			INVALIDATE_HANDLE(m_CurrentPipeline);
			INVALIDATE_HANDLE(m_CurrentMaterialHandle);
		}
	}

	void VulkanCommandBuffer::EndCurrentRenderPass()
	{
		if (HANDLE_IS_VALID(m_CurrentRenderPass) && m_isRecording)
		{
			if (m_Device->HasDynamicRenderingEnabled())
			{
				vkCmdEndRendering(m_Cmd);
			}
			else
			{
				vkCmdEndRenderPass(m_Cmd);
			}

			m_CurrentRenderPass = INVALID_HANDLE(RenderPassHandle);
		}
	}

	void VulkanCommandBuffer::DrawMesh(const RenderObjectInstance& renderElement, DescriptorAllocator* allocator)
	{
		CZUCH_BE_ASSERT(renderElement.IsValid(), "Render object instance passed to command buffer is invalid");

		Mesh* meshInstance = m_Device->AccessMesh(renderElement.mesh);
		if (meshInstance != nullptr)
		{
			auto pipelinePtr=BindMaterialInstance(renderElement.overrideMaterial, 0, allocator);

			TryBindMeshInstanceBuffers(meshInstance);

			auto vulkanPipeline = Internal_To_Pipeline(pipelinePtr);
			vkCmdPushConstants(m_Cmd, vulkanPipeline->pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, PUSH_CONSTANTS_SIZE, (void*)&renderElement.localToClipSpaceTransformation);
			auto indicesBuffer = m_Device->AccessBuffer(meshInstance->indicesHandle);
			if (indicesBuffer == nullptr)
			{
				LOG_BE_ERROR("{0} Failed to access indices buffer for mesh instance.", Tag);
				return;
			}

			auto indicesOffset = meshInstance->indicesHandle.offset >= 0 ? meshInstance->indicesHandle.offset : 0;
			indicesOffset /= sizeof(U32); // Vulkan uses indices offset in U32 units
			auto firstVertex = meshInstance->vertexBufferHandle.offset >= 0 ? meshInstance->vertexBufferHandle.offset : 0;
			firstVertex /= sizeof(Vertex); // Vulkan uses vertex offset in float3 units
			DrawIndexed(meshInstance->indicesHandle.size/sizeof(U32), indicesOffset, 1, 0,firstVertex);
		}
	}
	

	void VulkanCommandBuffer::TryBindMeshInstanceBuffers(Czuch::Mesh* meshInstance)
	{
		BindBuffer(meshInstance->vertexBufferHandle, 0,0,&m_CurrentVertexBuffer);
		if (meshInstance->IsValid() == false)
		{
			CZUCH_BE_ASSERT(false, "Mesh instance is invalid");
		}

		BindIndexBuffer(meshInstance->indicesHandle, 0);
	}

	void VulkanCommandBuffer::DrawFullScreenQuad(MaterialInstanceHandle mat, DescriptorAllocator* allocator)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(mat), "Material instance passed to command buffer is invalid");

		U32 passIndex = 0;
		MaterialInstance* materialInstance = m_Device->AccessMaterialInstance(mat);
		Material* material = m_Device->AccessMaterial(materialInstance->handle);

		auto& paramsDesc = materialInstance->params[passIndex].shaderParamsDesc;
		auto pipeline = material->pipelines[passIndex];
		auto& pipelineDesc = material->GetDesc().passesContainer.passes[passIndex];

		BindPipeline(pipeline);

		auto pipelinePtr = m_Device->AccessPipeline(pipeline);
		DescriptorWriter writer;

		for (int a = 0; a < pipelineDesc.layoutsCount; a++)
		{
			writer.Clear();
			auto descriptorLayout = pipelinePtr->layouts[a];
			auto layout = m_Device->AccessDescriptorSetLayout(descriptorLayout);
			auto descriptor = allocator->Allocate(paramsDesc[a], layout);

			for (int b = 0; b < layout->desc.bindingsCount; b++)
			{
				auto binding = layout->desc.bindings[b];
				if (binding.type == DescriptorType::UNIFORM_BUFFER)
				{
					writer.WriteBuffer(binding.index, m_Device->AccessBuffer(BufferHandle(paramsDesc[a].descriptors[b].resource)), binding.size, 0, binding.type);
				}
				else if (binding.type == DescriptorType::COMBINED_IMAGE_SAMPLER)
				{
					writer.WriteTexture(binding.index, m_Device->AccessTexture(TextureHandle(paramsDesc[a].descriptors[b].resource,AssetHandle())), DescriptorType::COMBINED_IMAGE_SAMPLER);
				}
				else if (binding.type == DescriptorType::STORAGE_BUFFER)
				{
					writer.WriteBuffer(binding.index, m_Device->AccessBuffer(BufferHandle(paramsDesc[a].descriptors[b].resource)), binding.size, 0, binding.type);
				}
				writer.UpdateSet(m_Device, descriptor);
			}

			BindDescriptorSet(descriptor, a, 1, nullptr, 0);
		}

	    DrawQuadInternal();
	}

	void VulkanCommandBuffer::BindPass(RenderPassHandle renderpass, FrameBufferHandle framebuffer)
	{
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(renderpass), "render pass passed to command buffer bind pass is null");
		CZUCH_BE_ASSERT(HANDLE_IS_VALID(framebuffer), "framebuffer passed to command buffer bind pass is null");

		if (renderpass.handle == m_CurrentRenderPass.handle && m_CurrentFrameBuffer.handle == framebuffer.handle)
		{
			return;
		}

		auto fb = m_Device->AccessFrameBuffer(framebuffer);
		auto render_pass = m_Device->AccessRenderPass(renderpass);

		if (m_Device->HasDynamicRenderingEnabled())
		{
			auto& desc = fb->desc;
			auto& renderPassDesc = render_pass->desc;
			for (U32 a = 0; a < desc.renderTargetsCount; ++a) {
				Texture* texture = m_Device->AccessTexture(desc.renderTextures[a]);
				VkAttachmentLoadOp color_op;
				switch (renderPassDesc.colorAttachments[a].loadOp) {
				case AttachmentLoadOp::LOAD:
					color_op = VK_ATTACHMENT_LOAD_OP_LOAD;
					break;
				case AttachmentLoadOp::CLEAR:
					color_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
					break;
				default:
					color_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					break;
				}

				VkRenderingAttachmentInfoKHR& colorAttachmentInfo = m_ColorAttachmentsInfo[a];
				colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
				colorAttachmentInfo.imageView = Internal_to_Texture(texture)->imageView;
				colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
				colorAttachmentInfo.loadOp = color_op;
				colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				colorAttachmentInfo.clearValue = renderPassDesc.colorAttachments[a].loadOp == AttachmentLoadOp::CLEAR ? m_ClearValueColor : VkClearValue{};
			}


			VkRenderingAttachmentInfoKHR depthAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };

			bool hasDepthAttachment = HANDLE_IS_VALID(fb->desc.depthStencilTexture);

			if (hasDepthAttachment) {
				Texture* texture = m_Device->AccessTexture(fb->desc.depthStencilTexture);

				VkAttachmentLoadOp depth_op;
				switch (render_pass->desc.depthLoadOp) {
				case AttachmentLoadOp::LOAD:
					depth_op = VK_ATTACHMENT_LOAD_OP_LOAD;
					break;
				case AttachmentLoadOp::CLEAR:
					depth_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
					break;
				default:
					depth_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					break;
				}

				depthAttachmentInfo.imageView = Internal_to_Texture(texture)->imageView;
				depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
				depthAttachmentInfo.loadOp = depth_op;
				depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				depthAttachmentInfo.clearValue = render_pass->desc.depthLoadOp == AttachmentLoadOp::CLEAR ? m_ClearValueDepth : VkClearValue{ };
			}

			VkRenderingInfoKHR renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO_KHR };
			renderingInfo.flags = 0;
			renderingInfo.renderArea = { 0, 0,desc.width,desc.height };
			renderingInfo.layerCount = 1;
			renderingInfo.viewMask = 0;
			renderingInfo.colorAttachmentCount = desc.renderTargetsCount;
			renderingInfo.pColorAttachments = desc.renderTargetsCount > 0 ? &m_ColorAttachmentsInfo[0] : nullptr;
			renderingInfo.pDepthAttachment = hasDepthAttachment ? &depthAttachmentInfo : nullptr;
			renderingInfo.pStencilAttachment = nullptr;

			vkCmdBeginRendering(m_Cmd, &renderingInfo);
		}
		else
		{
			auto fbNative = Internal_to_Framebuffer(fb);
			VkExtent2D extent{};
			extent.width = fbNative->createInfo.width;
			extent.height = fbNative->createInfo.height;

			auto& renderPassDesc = render_pass->desc;
			bool hasDepthAttachment = HANDLE_IS_VALID(fb->desc.depthStencilTexture);

			VkClearValue clearValues[] = { m_ClearValueColor,m_ClearValueDepth };
			int count = 0;
			VkClearValue* clearTarget = nullptr;
			if ((hasDepthAttachment && renderPassDesc.attachmentsCount>0) || fb->desc.isFinalFrameBuffer)
			{
				count = 2;
				clearTarget = clearValues;
			}
			else if (renderPassDesc.attachmentsCount > 0)
			{
				count = 1;
				clearTarget = &clearValues[0];
			}
			else if (hasDepthAttachment)
			{
				count = 1;
				clearTarget = &clearValues[1];
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = Internal_to_RenderPass(m_Device->AccessRenderPass(renderpass))->renderPass;
			renderPassInfo.framebuffer = fbNative->framebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = extent;
			renderPassInfo.clearValueCount = count;
			renderPassInfo.pClearValues = clearTarget;

			vkCmdBeginRenderPass(m_Cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		}
		m_CurrentFrameBuffer = framebuffer;
		m_CurrentRenderPass = renderpass;

	}

	void VulkanCommandBuffer::BindPipeline(PipelineHandle pipeline)
	{
		if (!HANDLE_IS_VALID(pipeline))
		{
			return;
		}

		if (pipeline.handle == m_CurrentPipeline.handle)
		{
			return;
		}

		CZUCH_BE_ASSERT(pipeline.handle != Invalid_Handle_Id, "Trying to bind invalid pipeline!");

		auto pp = m_Device->AccessPipeline(pipeline);
		VkPipeline pipelineVk = Internal_To_Pipeline(pp)->pipeline;
		vkCmdBindPipeline(m_Cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineVk);
		m_CurrentPipeline = pipeline;
	}

	 Pipeline* VulkanCommandBuffer::BindMaterialInstance(MaterialInstanceHandle materialHandle, U32 passIndex, DescriptorAllocator* allocator)
	{
		 MaterialInstance* materialInstance = m_Device->AccessMaterialInstance(materialHandle);
		 Material* material = m_Device->AccessMaterial(materialInstance->handle);
		 auto& paramsDesc = materialInstance->params[passIndex].shaderParamsDesc;
		 auto pipeline = material->pipelines[passIndex];
		 auto pipelinePtr = m_Device->AccessPipeline(pipeline);

		if(m_CurrentMaterialHandle.handle== materialHandle.handle)
		{
			return pipelinePtr;
		}
		m_CurrentMaterialHandle = materialHandle;;
		auto& pipelineDesc = material->GetDesc().passesContainer.passes[passIndex];
		BindPipeline(pipeline);

		bool isDirty =  materialInstance->IsDirty();
		materialInstance->ClearDirty();

		for (int a = 0; a < pipelineDesc.layoutsCount; a++)
		{
			if (paramsDesc[a].isBindlessTexturesSet)
			{
				//Bind bindless descriptor set
				auto descriptorLayout = pipelinePtr->layouts[a];
				auto layout = m_Device->AccessDescriptorSetLayout(descriptorLayout);

				BindDescriptorSet(m_Device->GetBindlessTexturesDescriptorSet(), a, 1, nullptr, 0);
				continue;
			}

			if (paramsDesc[a].descriptorsCount <= 0)
			{
				continue;
			}

			auto descriptorLayout = pipelinePtr->layouts[a];
			auto layout = m_Device->AccessDescriptorSetLayout(descriptorLayout);
			auto& layoutDesc = pipelineDesc.layouts[a];


			if (paramsDesc[a].currentDescriptor == nullptr)
			{
				paramsDesc[a].currentDescriptor = allocator->Allocate(paramsDesc[a], layout);
			}

			if (isDirty)
			{
				m_Writer.Clear();
				for (int b = 0; b < layoutDesc.bindingsCount; b++)
				{
					auto binding = layoutDesc.bindings[b];
					if (binding.type == DescriptorType::UNIFORM_BUFFER)
					{
						m_Writer.WriteBuffer(binding.index, m_Device->AccessBuffer(BufferHandle(paramsDesc[a].descriptors[b].resource)), paramsDesc[a].descriptors[b].size, 0, binding.type);
					}
					else if (binding.type == DescriptorType::COMBINED_IMAGE_SAMPLER)
					{
						m_Writer.WriteTexture(binding.index, m_Device->AccessTexture({ paramsDesc[a].descriptors[b].resource,AssetHandle() }), DescriptorType::COMBINED_IMAGE_SAMPLER);
					}
					else if (binding.type == DescriptorType::STORAGE_BUFFER)
					{
						if (paramsDesc[a].descriptors[b].resource != -1)
						{
							auto buffer = m_Device->AccessBuffer(BufferHandle(paramsDesc[a].descriptors[b].resource));
							if (buffer != nullptr)
							{
								m_Writer.WriteBuffer(binding.index, buffer, paramsDesc[a].descriptors[b].size, 0, binding.type);
							}
						}

					}
					m_Writer.UpdateSet(m_Device, paramsDesc[a].currentDescriptor);
				}
			}

			BindDescriptorSet(paramsDesc[a].currentDescriptor, a, 1, nullptr, 0);
		}

		return pipelinePtr;
	}

	void VulkanCommandBuffer::BindBuffer(BufferHandle buffer, U32 binding, U32 offset, BufferHandle* prevBuffer)
	{
		CZUCH_BE_ASSERT(buffer.handle != Invalid_Handle_Id, "Trying to bind invalid buffer!");
	
		if (prevBuffer != nullptr)
		{
			if (prevBuffer->handle == buffer.handle)
			{
				return;
			}
			*prevBuffer = buffer;
		}
		VkBuffer vertexBuffers[] = { Internal_to_Buffer(m_Device->AccessBuffer(buffer))->buffer };
		VkDeviceSize offsets[] = { offset };
		vkCmdBindVertexBuffers(m_Cmd, binding, 1, vertexBuffers, offsets);
	}

	void VulkanCommandBuffer::BindIndexBuffer(BufferHandle buffer, U32 offset = 0)
	{
		if (buffer.handle == m_CurrentIndexBuffer.handle)
		{
			return;
		}
		m_CurrentIndexBuffer = buffer;
		VkDeviceSize offsets = offset;
		vkCmdBindIndexBuffer(m_Cmd, Internal_to_Buffer(m_Device->AccessBuffer(buffer))->buffer, offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanCommandBuffer::BindDescriptorSet(DescriptorSet* descriptor, U16 setIndex, U32 num, U32* offsets, U32 num_offsets)
	{
		auto pp = m_Device->AccessPipeline(m_CurrentPipeline);
		if (pp == nullptr)
		{
			LOG_BE_ERROR("{0} Failed to bind descriptor set, pipeline is invalid", Tag);
			return;
		}
		CZUCH_BE_ASSERT(pp != nullptr, "No pipeline bound to command buffer");
		CZUCH_BE_ASSERT(descriptor != nullptr, "Descriptor set passed to command buffer is null")
		CZUCH_BE_ASSERT(descriptor->descriptorSet != VK_NULL_HANDLE, "Descriptor set handle is VK_NULL_HANDLE");

		auto vulkanPipeline = Internal_To_Pipeline(pp);
		vkCmdBindDescriptorSets(m_Cmd, ConvertBindPoint(pp->m_desc.bindPoint), vulkanPipeline->pipelineLayout, setIndex, num, &descriptor->descriptorSet, num_offsets, offsets);
	}

	void VulkanCommandBuffer::BindDescriptorSet(ParamSetUpdateControl* control, U16 setIndex, U32 num, U32* offsets, U32 num_offsets)
	{
		CZUCH_BE_ASSERT(control != nullptr, "ParamSetUpdateControl is null");
		ParamSetVulkanUpdateControl* vulkanControl = static_cast<ParamSetVulkanUpdateControl*>(control);
		BindDescriptorSet(vulkanControl->descriptorSet[vulkanControl->GetCurrentFrameIndex()], setIndex, num, offsets, num_offsets);
	}

	void VulkanCommandBuffer::SetClearColor(float r, float g, float b, float a)
	{
		m_ClearValueColor.color = VkClearColorValue{ r,g,b,a };
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
		viewportVk.height = -viewport.height;//[Vulkan] Y axis is inverted
		viewportVk.minDepth = viewport.minDepth;
		viewportVk.maxDepth = viewport.maxDepth;
		vkCmdSetViewport(m_Cmd, 0, 1, &viewportVk);
	}

	void VulkanCommandBuffer::SetScrissors(ScissorsDesc scissors)
	{
		VkRect2D scissorVk{};
		scissorVk.offset = { scissors.offsetX, scissors.offsetY };
		scissorVk.extent.width = scissors.width;
		scissorVk.extent.height = scissors.height;
		vkCmdSetScissor(m_Cmd, 0, 1, &scissorVk);
	}

	void VulkanCommandBuffer::Draw(U32 vertexCount, U32 firstVertex, U32 instanceCount, U32 firstInstance)
	{
		vkCmdDraw(m_Cmd, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexed(U32 indicesCount, U32 firstIndex, U32 instanceCount, U32 firstnstance, U32 vertexOffset)
	{
		vkCmdDrawIndexed(m_Cmd, indicesCount, instanceCount, firstIndex, vertexOffset, firstnstance);
	}

	void VulkanCommandBuffer::DrawIndirectIndexedWithData(IndirectDrawForCommandBufferData* data, DescriptorAllocator* allocator)
	{	
		CZUCH_BE_ASSERT(data != nullptr, "Indirect draw data is null");
		CZUCH_BE_ASSERT(data->buffer != nullptr, "Indirect draw buffer is null");

		CZUCH_BE_ASSERT(HANDLE_IS_VALID(data->material), "Material instance passed to command buffer is invalid");

		auto mat = data->material;
		U32 passIndex = 0;
		MaterialInstance* materialInstance = m_Device->AccessMaterialInstance(mat);
		CZUCH_BE_ASSERT(materialInstance != nullptr, "Material instance passed to command buffer is null");
		Material* material = m_Device->AccessMaterial(materialInstance->handle);

		auto& paramsDesc = materialInstance->params[passIndex].shaderParamsDesc;
		auto pipeline = material->pipelines[passIndex];
		auto& pipelineDesc = material->GetDesc().passesContainer.passes[passIndex];

		BindPipeline(pipeline);

		auto pipelinePtr = m_Device->AccessPipeline(pipeline);
		DescriptorWriter writer;

		//set scene data for material
		paramsDesc[0].descriptors[0].resource = data->sceneDataBuffer.handle;
		
		//set instances count in material
		paramsDesc[data->set].descriptors[data->binding].resource = data->instancesBuffer.handle;

		for (int a = 0; a < pipelineDesc.layoutsCount; a++)
		{
			writer.Clear();
			auto descriptorLayout = pipelinePtr->layouts[a];
			auto layout = m_Device->AccessDescriptorSetLayout(descriptorLayout);
			auto descriptor = allocator->Allocate(paramsDesc[a], layout);

			for (int b = 0; b < layout->desc.bindingsCount; b++)
			{
				auto binding = layout->desc.bindings[b];
				if (binding.type == DescriptorType::UNIFORM_BUFFER)
				{
					writer.WriteBuffer(binding.index, m_Device->AccessBuffer(BufferHandle(paramsDesc[a].descriptors[b].resource)), binding.size, 0, binding.type);
				}
				else if (binding.type == DescriptorType::COMBINED_IMAGE_SAMPLER)
				{
					writer.WriteTexture(binding.index, m_Device->AccessTexture(TextureHandle(paramsDesc[a].descriptors[b].resource, AssetHandle())), DescriptorType::COMBINED_IMAGE_SAMPLER);
				}
				else if (binding.type == DescriptorType::STORAGE_BUFFER)
				{
					U32 size = binding.size;

					if (a == data->set && b == data->binding)
					{
						size = data->instancesSize;
					}

					writer.WriteBuffer(binding.index, m_Device->AccessBuffer(BufferHandle(paramsDesc[a].descriptors[b].resource)), size==0? VK_WHOLE_SIZE:size, 0, binding.type);
				}
				writer.UpdateSet(m_Device, descriptor);
			}

			BindDescriptorSet(descriptor, a, 1, nullptr, 0);
		}

		//set vertex and index buffer
		BindBuffer(data->vertexBuffer, 0, 0,nullptr);

		if (HANDLE_IS_VALID(data->indexBuffer))
		{
			BindIndexBuffer(data->indexBuffer, 0);
		}

		auto indirectBuffer=m_Device->AccessBuffer(data->indirectDrawsCommandsBuffer);
		CZUCH_BE_ASSERT(indirectBuffer != nullptr, "Indirect draw buffer is null");
		VkBuffer buffer = Internal_to_Buffer(indirectBuffer)->buffer;
		vkCmdDrawIndexedIndirect(m_Cmd, buffer, data->indirectDrawsCommandsOffset, 1, sizeof(VkDrawIndexedIndirectCommand));
	}

	void VulkanCommandBuffer::BeginDynamicRenderPassForMainPass(VkImageView colorView, VkImageView depthView, U32 width, U32 height)
	{
		VkRenderingAttachmentInfoKHR colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachment.imageView = colorView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue.color = m_ClearValueColor.color;

		VkRenderingAttachmentInfoKHR depthStencilAttachment{};
		depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthStencilAttachment.imageView = depthView;
		depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthStencilAttachment.clearValue.depthStencil = m_ClearValueDepth.depthStencil;

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

	void VulkanCommandBuffer::EndDynamicRenderPassForMainPass()
	{
		vkCmdEndRendering(m_Cmd);
	}

	void VulkanCommandBuffer::DrawQuadInternal()
	{
		vkCmdDraw(m_Cmd, 6, 1, 0, 0);
	}

}
