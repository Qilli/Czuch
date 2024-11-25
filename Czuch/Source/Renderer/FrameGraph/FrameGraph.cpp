#include "czpch.h"
#include "FrameGraph.h"
#include "../GraphicsDevice.h"
#include "../CommandBuffer.h"
#include "../RenderPassControl.h"

namespace Czuch
{
	void FrameGraph::Init(GraphicsDevice* device,Renderer* renderer)
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

	void FrameGraph::Execute(GraphicsDevice* device, CommandBuffer* cmd)
	{
		for (U32 i = 0; i < m_Nodes.m_Nodes.size(); i++)
		{
			auto& node = m_Nodes.m_Nodes[i];
			cmd->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			cmd->SetDepthStencil(1.0f, 0);

			U32 width = 0;
			U32 height = 0;

			for (int a = 0; a < node.inputs.size(); a++)
			{
				auto& input = m_Resources.GetResource(node.inputs[a]);

				if (input.type == FrameGraphResourceType::Texture)
				{
					device->TransitionImageLayout(input.info.texture.texture, ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::SHADER_READ_ONLY_OPTIMAL, 0, 1, IsDepthFormat(input.info.texture.format));
				}
				else if (input.type == FrameGraphResourceType::Attachment)
				{
					auto res=GetResource(input.output_target);
					auto texture = device->AccessTexture(res.info.texture.texture);
					width = texture->desc.width;
					height = texture->desc.height;
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
					device->TransitionImageLayout(output.info.texture.texture, ImageLayout::UNDEFINED, isDepth ? (isDepthStencil?ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:ImageLayout::DEPTH_ATTACHMENT_OPTIMAL) : ImageLayout::COLOR_ATTACHMENT_OPTIMAL, 0, 1, isDepth);
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

			renderPassControl->PreDraw(cmd,m_Renderer);

			cmd->BindPass(node.renderPass, node.frameBuffer);

			renderPassControl->Execute(cmd);

			renderPassControl->PostDraw(cmd,m_Renderer);

			cmd->EndCurrentRenderPass();
		}
	}

	void FrameGraph::ResizeNode(FrameGraphNode node, U32 width, U32 height)
	{
		node.Resize(m_Device, width, height);
		//resize all textures created by this node
		for (int i = 0; i < node.outputs.size(); i++)
		{
			auto& output = m_Resources.GetResource(node.outputs[i]);
			if (output.type == FrameGraphResourceType::Attachment)
			{
				m_Device->ResizeTexture(output.info.texture.texture, width, height);
			}
		}
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
		auto& node = m_Nodes.m_Nodes.back();
		return node.GetFirstColorAttachment(this);
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
		return TextureHandle{ Invalid_Handle_Id };
	}

}

