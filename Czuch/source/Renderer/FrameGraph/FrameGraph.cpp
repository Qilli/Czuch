#include "czpch.h"
#include "FrameGraph.h"
#include "../GraphicsDevice.h"
#include "../CommandBuffer.h"
#include "../RenderPassControl.h"

namespace Czuch
{
	void FrameGraph::Init(GraphicsDevice* device,Renderer* renderer )
	{
		m_Device = device;
		m_Renderer= renderer;
		m_Nodes.Init(device);
		m_Resources.Init(device);
	}

	void FrameGraph::Release()
	{
		m_Nodes.Release();
		m_Resources.Release();
	}

	FrameGraphNodeHandle FrameGraph::CreateNewNode()
	{
		return m_Nodes.CreateNewNode();
	}

	FrameGraphResourceHandle FrameGraph::CreateNewResource()
	{
		return m_Resources.CreateNewResource();
	}

	void FrameGraph::AfterSystemInit()
	{
		for (U32 i = 0; i < m_Nodes.m_Nodes.size(); i++)
		{
			auto& node = m_Nodes.m_Nodes[i];
			node.renderPassControl->Init();
		}
	}

	void FrameGraph::Execute(GraphicsDevice* device, CommandBuffer* cmd)
	{

		for (U32 i = 0; i < m_Nodes.m_Nodes.size(); i++)
		{
			auto& node = m_Nodes.m_Nodes[i];

			if (node.renderPassControl->IsActive() == false)
			{
				continue;
			}

			if (m_UseClearColor)
			{
				cmd->SetClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, 1.0f);
			}
			else
			{
				cmd->SetClearColor(node.clearColor.r, node.clearColor.g, node.clearColor.b, 1.0f);
			}
			cmd->SetDepthStencil(1.0f, 0);

			U32 width = 0;
			U32 height = 0;

			for (int a = 0; a < node.inputs.size(); a++)
			{
				auto& input = m_Resources.GetResource(node.inputs[a]);

				if (input.type == FrameGraphResourceType::Texture)
				{
					auto &res = GetResource(input.output_target);
					auto texture = device->AccessTexture(res.info.texture.texture);
					bool isDepth = IsDepthFormat(res.info.texture.format);
					auto targetLayout = ImageLayout::SHADER_READ_ONLY_OPTIMAL;
					device->TryTransitionImageLayout(cmd, res.info.texture.texture, targetLayout, 0, 1);
				}
				else if (input.type == FrameGraphResourceType::Attachment)
				{
					auto& res = GetResource(input.output_target);
					auto texture = device->AccessTexture(res.info.texture.texture);
					width = texture->desc.width;
					height = texture->desc.height;

					bool isDepth = IsDepthFormat(res.info.texture.format);
					bool isDepthStencil = IsDepthFormatWithStencil(res.info.texture.format);
					auto targetLayout = isDepthStencil ? ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL : (isDepth ? ImageLayout::DEPTH_ATTACHMENT_OPTIMAL : ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
					device->TryTransitionImageLayout(cmd, res.info.texture.texture, targetLayout, 0, 1);
				}
			}

			for (int a = 0; a < node.outputs.size(); a++)
			{
				auto& output = m_Resources.GetResource(node.outputs[a]);

				if (output.type == FrameGraphResourceType::Attachment)
				{ 
					auto texture = device->AccessTexture(output.info.texture.texture);
					width = texture->desc.width;
					height = texture->desc.height;
					bool isDepth = IsDepthFormat(output.info.texture.format);
					bool isDepthStencil = IsDepthFormatWithStencil(output.info.texture.format);
					auto targetLayout = isDepthStencil ? ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL : (isDepth ? ImageLayout::DEPTH_ATTACHMENT_OPTIMAL : ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
					device->TryTransitionImageLayout(cmd, output.info.texture.texture, targetLayout, 0, 1);
				}
			}

			ScissorsDesc scissor{ 0, 0,(U16)width, (U16)height };
			cmd->SetScrissors(scissor);

			ViewportDesc viewport{ };
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = (F32)width;
			viewport.height = (F32)height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			cmd->SetViewport(viewport);

			auto renderPassControl = node.renderPassControl;

			renderPassControl->PreDraw(cmd, m_Renderer);

			cmd->BindPass(node.renderPass, node.frameBuffer);
			renderPassControl->BindInputTextures(device, &node);

			renderPassControl->Execute(cmd);

			renderPassControl->PostDraw(cmd, m_Renderer);

			cmd->EndCurrentRenderPass();
		}

		//transition results to shader read only
		for (U32 i = 0; i < m_Nodes.m_Nodes.size(); i++)
		{
			auto& node = m_Nodes.m_Nodes[i];
			auto renderPassControl = node.renderPassControl;

			renderPassControl->TransitionResultsToShaderReadOnly(cmd);
		}
	}

	void FrameGraph::ResizeNode(FrameGraphNode node, U32 width, U32 height)
	{
		for (int i = 0; i < node.outputs.size(); i++)
		{
			auto& output = m_Resources.GetResource(node.outputs[i]);
			if (output.type == FrameGraphResourceType::Attachment)
			{
				m_Device->ResizeTexture(output.info.texture.texture, width, height);
			}
		}

		node.Resize(m_Device, width, height);
	}

	void* FrameGraph::GetRenderPassResult(RenderPassType type)
	{
		for (int i = 0; i < m_Nodes.m_Nodes.size(); i++)
		{
			auto& node = m_Nodes.m_Nodes[i];
			if (node.renderPassControl->GetType() == type)
			{
				return node.renderPassControl->GetRenderPassResult();
			}
		}
		return nullptr;
	}

	void* FrameGraph::GetFinalRenderPassResult()
	{
		return m_Nodes.m_Nodes.back().renderPassControl->GetRenderPassResult();
	}

	RenderPassControl* FrameGraph::GetRenderPassControlByType(RenderPassType type)
	{
		for (int i = 0; i < m_Nodes.m_Nodes.size(); i++)
		{
			auto& node = m_Nodes.m_Nodes[i];
			if (node.renderPassControl->GetType() == type)
			{
				return node.renderPassControl;
			}
		}
		return nullptr;
	}

	bool FrameGraph::HasRenderPass(RenderPassType type)
	{
		for (int i = 0; i < m_Nodes.m_Nodes.size(); i++)
		{
			auto& node = m_Nodes.m_Nodes[i];
			if (node.renderPassControl->GetType() == type)
			{
				return true;
			}
		}
		return false;
	}

	void FrameGraph::ResizeRenderPasses(U32 width, U32 height, bool windowSizeChanged)
	{
		//resize every node render pass control
		for (int i = 0; i < m_Nodes.m_Nodes.size(); i++)
		{
			auto& node = m_Nodes.m_Nodes[i];
			if (node.renderPassControl->HandleWindowResize())
			{
				if (windowSizeChanged)
				{
					ResizeNode(node, width, height);
				}
			}
			else if(!windowSizeChanged)
			{
				ResizeNode(node, width, height);
			}
		}
	}

	TextureHandle FrameGraph::GetFinalTexture()
	{
		//return last color attachment
		FrameGraphNode* node = nullptr;

		for (int i = m_Nodes.m_Nodes.size()-1; i>=0; i--)
		{
			auto& n = m_Nodes.m_Nodes[i];
			if (n.renderPassControl->IsActive())
			{
				node = &n;
				break;
			}
		}

		CZUCH_BE_ASSERT(node, "Final render pass not found");
		return node->GetFirstColorAttachment(this);
	}

	void FrameGraph::ReleaseDependencies()
	{
		m_Nodes.ReleaseDependencies();
	}

	void* FrameGraph::GetRenderPassResultAt(U32 renderPassIndex)
	{
		CZUCH_BE_ASSERT(renderPassIndex < m_Nodes.m_Nodes.size(), "Render pass index out of range");
		auto& node = m_Nodes.m_Nodes[renderPassIndex];
		return node.renderPassControl->GetRenderPassResult();
	}

	void FrameGraphNodesContainer::Init(GraphicsDevice* dev)
	{
		device = dev;
		m_Nodes.reserve(20);
	}

	void FrameGraphNodesContainer::Release()
	{
		for (U32 i = 0; i < m_Nodes.size(); i++)
		{
			m_Nodes[i].Release(device);
		}
		m_Nodes.clear();
	}

	void FrameGraphNodesContainer::ReleaseDependencies()
	{
		for (U32 i = 0; i < m_Nodes.size(); i++)
		{
			m_Nodes[i].renderPassControl->ReleaseDependencies();
		}
	}

	FrameGraphNode& FrameGraphNodesContainer::GetNode(FrameGraphNodeHandle handle)
	{
		assert(handle.handle < m_Nodes.size() && handle.handle >= 0);
		return m_Nodes[handle.handle];
	}

	FrameGraphNodeHandle FrameGraphNodesContainer::CreateNewNode()
	{
		auto& node = m_Nodes.emplace_back();
		return FrameGraphNodeHandle{ (I32)m_Nodes.size() - 1 };
	}

	void FrameGraphResourcesContainer::Init(GraphicsDevice* device)
	{

	}

	void FrameGraphResourcesContainer::Release()
	{
		resources.clear();
		resourceMap.clear();
	}

	FrameGraphResourceHandle FrameGraphResourcesContainer::CreateNewResource()
	{
		auto& resource = resources.emplace_back();
		return FrameGraphResourceHandle{ (I32)resources.size() - 1 };
	}

	FrameGraphResource& FrameGraphResourcesContainer::GetResource(FrameGraphResourceHandle handle)
	{
		assert(handle.handle < resources.size() && handle.handle >= 0);
		return resources[handle.handle];
	}

	void FrameGraphNode::Release(GraphicsDevice* device)
	{
		if (renderPassControl)
		{
			delete renderPassControl;
		}
		device->Release(frameBuffer);
	}

	void FrameGraphNode::Resize(GraphicsDevice* device, U32 width, U32 height)
	{
		device->ResizeFrameBuffer(frameBuffer, width, height);
		if (renderPassControl)
		{
			renderPassControl->Resize(width, height);
		}
	}

	TextureHandle FrameGraphNode::GetFirstColorAttachment(FrameGraph* fgraph)
	{
		//go through outputs and find first color attachment
		for (int i = 0; i < outputs.size(); i++)
		{
			auto outputHandle = outputs[i];
			//get resource using out handle 
			auto& output = fgraph->GetResource(outputHandle);
			if (output.type == FrameGraphResourceType::Attachment)
			{
				//check if it has color format
				if (!IsDepthFormat(output.info.texture.format))
				{
					return output.info.texture.texture;
				}
			}
		}


		//if not found check inputs
		for (int i = 0; i < inputs.size(); i++)
		{
			auto inputHandle = inputs[i];
			//get resource using out handle 
			auto& input = fgraph->GetResource(inputHandle);
			if (input.type == FrameGraphResourceType::Attachment)
			{
				//check if it has color format
				if (!IsDepthFormat(input.info.texture.format))
				{
					auto& res = fgraph->GetResource(input.output_target);
					return res.info.texture.texture;
				}
			}
		}

		return TextureHandle(Invalid_Handle_Id);
	}

	TextureHandle FrameGraphNode::GetDepthAttachment(FrameGraph* fgraph)
	{
		//go through outputs and find first depth attachment
		for (int i = 0; i < outputs.size(); i++)
		{
			auto outputHandle = outputs[i];
			//get resource using out handle 
			auto& output = fgraph->GetResource(outputHandle);
			if (output.type == FrameGraphResourceType::Attachment)
			{
				//check if it has depth format
				if (IsDepthFormat(output.info.texture.format))
				{
					return output.info.texture.texture;
				}
			}
		}
		return TextureHandle{ Invalid_Handle_Id };
	}

}

