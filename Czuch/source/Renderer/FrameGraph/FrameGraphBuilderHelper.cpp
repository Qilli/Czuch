#include"czpch.h"
#include"FrameGraphBuilderHelper.h"
#include"../Graphics.h"
#include"../GraphicsDevice.h"
#include"../RenderPassControl.h"
#include<stack>

namespace Czuch
{
	void FrameGraphBuilderHelper::Init(GraphicsDevice* device,Renderer*renderer)
	{
		m_Inited = true;
		m_Device = device;
		m_NodesData.reserve(20);
		m_FrameGraph = nullptr;
	}

	void FrameGraphBuilderHelper::Release()
	{
		m_Inited = false;
	}

	void FrameGraphBuilderHelper::BeginNewNode(const CzuchStr& name)
	{
		m_CurrentNodeData.Clear();
		m_CurrentNodeData.name = std::move(name);
	}

	void FrameGraphBuilderHelper::AddInput(FrameGraphResourceInputCreation input)
	{
		m_CurrentNodeData.inputs.push_back(input);
	}

	void FrameGraphBuilderHelper::AddOutput(FrameGraphResourceOutputCreation output)
	{
		m_CurrentNodeData.outputs.push_back(output);
	}

	void FrameGraphBuilderHelper::SetClearColor(const Vec3& color)
	{
		m_CurrentNodeData.clearColor = color;
	}

	void FrameGraphBuilderHelper::SetIgnoreResize(bool ignoreResize)
	{
		m_CurrentNodeData.ignoreResize = ignoreResize;
	}

	void FrameGraphBuilderHelper::SetRenderPassControl(RenderPassControl* control)
	{
		m_CurrentNodeData.control = control;
	}

	void FrameGraphBuilderHelper::EndNode()
	{
		m_NodesData.push_back(m_CurrentNodeData);
	}

	void TopologicalDFS(FrameGraph& frameGraph, FrameGraphNodeHandle handle, Array<I8>& visited, Array<FrameGraphNodeHandle>& m_SortedNodes)
	{
		visited[handle.handle] = 1;
		auto& node = frameGraph.GetNode(handle);
		for (auto& edge : node.edges)
		{
			if (visited[edge.handle] == 0)
			{
				TopologicalDFS(frameGraph, edge, visited, m_SortedNodes);
			}
		}
		m_SortedNodes.push_back(handle);
	}

	void FrameGraphBuilderHelper::Build(FrameGraph* graph)
	{
		m_FrameGraph = graph;
		tempNodes.reserve(m_NodesData.size());
		for (auto& nodeData : m_NodesData)
		{
			tempNodes.push_back(CreateNode(nodeData));
		}

		//compute edges
		for (U32 i = 0; i < tempNodes.size(); ++i) {
			FrameGraphNode* node = &m_FrameGraph->GetNode(tempNodes[i]);
			ComputeEdgesFor(node, i);
		}


		//topological sort
		m_SortedNodes.reserve(tempNodes.size());
		Array<I8> visited(tempNodes.size(), 0);
		for (U32 i = 0; i < tempNodes.size(); ++i)
		{
			if (visited[i] == 0)
			{
				TopologicalDFS(*m_FrameGraph, tempNodes[i], visited, m_SortedNodes);
			}
		}
		m_FrameGraph->m_SortedNodes = std::move(m_SortedNodes);

		//revert sorted nodes
		std::reverse(m_FrameGraph->m_SortedNodes.begin(), m_FrameGraph->m_SortedNodes.end());

		//compute references
		for (U32 i = 0; i < tempNodes.size(); ++i) {
			FrameGraphNode* node = &m_FrameGraph->GetNode(tempNodes[i]);
			for (U32 r = 0; r < node->inputs.size(); ++r) {
				FrameGraphResource* resource = &m_FrameGraph->GetResource(node->inputs[r]);
				FrameGraphResource* output_resource = &m_FrameGraph->GetResource(resource->output_target);
				output_resource->refCount++;
			}
		}

		//[TODO] HERE IS THE PLACE TO ADD SUPPORT FOR ALIASING RESOURCES(SHARING RESOURCES USING VMA AIASING) FOR OPTIMIZATION
		/*U32 resourceCount = m_FrameGraph->resources.resources.size();
		Array<FrameGraphNodeHandle> allocations(resourceCount,{Invalid_Handle_Id});
		Array<FrameGraphNodeHandle> deallocations(resourceCount,{Invalid_Handle_Id});
		Array<TextureHandle> texturesFreeToUse(resourceCount*0.5f);

		for (U32 i = 0; i < m_FrameGraph->sortedNodes.size(); ++i) {
			FrameGraphNode* node = &m_FrameGraph->GetNode(m_FrameGraph->sortedNodes[i]);
			for (U32 r = 0; r < node->outputs.size(); ++r) {
				FrameGraphResource* resource = &m_FrameGraph->GetResource(node->outputs[r]);
				if (resource->type == FrameGraphResourceType::Reference) {
					continue;
				}

				if (texturesFreeToUse.size()>0) {
					//reuse texture
				}
				else {
					//create new texture
				}
			}
		}*/

		for (U32 i = 0; i < m_FrameGraph->m_SortedNodes.size(); ++i) {
			FrameGraphNode* node = &m_FrameGraph->GetNode(m_FrameGraph->m_SortedNodes[i]);
			for (U32 r = 0; r < node->outputs.size(); ++r) {
				FrameGraphResource* resource = &m_FrameGraph->GetResource(node->outputs[r]);
				if (resource->type == FrameGraphResourceType::Reference) {
					continue;
				}

				//create textures and buffers
				if (resource->type == FrameGraphResourceType::Attachment) {

					SamplerDesc samplerDesc{};
					samplerDesc.magFilter = TextureFilter::LINEAR;
					samplerDesc.minFilter = TextureFilter::LINEAR;
					samplerDesc.addressModeU = TextureAddressMode::EDGE_CLAMP;
					samplerDesc.addressModeV = TextureAddressMode::EDGE_CLAMP;
					samplerDesc.addressModeW = TextureAddressMode::EDGE_CLAMP;

					TextureDesc texDesc{};
					texDesc.type = TextureDesc::Type::TEXTURE_2D;
					texDesc.width = resource->info.texture.width;
					texDesc.height = resource->info.texture.height;
					texDesc.depth = 1;
					texDesc.format = resource->info.texture.format;
					texDesc.samplerDesc = samplerDesc;
					texDesc.sample_count = 1;
					texDesc.layoutInfo.SetAllTo(ImageLayout::UNDEFINED);
					texDesc.mip_levels = 1;
					texDesc.name = resource->name.c_str();
					

					if (HAS_FLAG(resource->info.texture.usage, ImageUsageFlag::DEPTH_STENCIL_ATTACHMENT))
					{
						texDesc.usageFlags = ImageUsageFlag::DEPTH_STENCIL_ATTACHMENT | ImageUsageFlag::SAMPLED;
						texDesc.aspectFlags = ImageAspectFlag::DEPTH | ImageAspectFlag::STENCIL;
						texDesc.resourceType = ResourceState::DEPTHSTENCIL;
					}
					else
					{
						texDesc.usageFlags = ImageUsageFlag::COLOR_ATTACHMENT | ImageUsageFlag::SAMPLED;
						texDesc.aspectFlags = ImageAspectFlag::COLOR;
						texDesc.resourceType = ResourceState::RENDERTARGET;
					}

					resource->info.texture.texture = m_Device->CreateTexture(&texDesc);
					resource->info.texture.texture = m_Device->BindGlobalTexture(resource->info.texture.texture);
				}
				else if (resource->type == FrameGraphResourceType::Buffer) {
					BufferDesc info;
					info.size = resource->info.buffer.size;
					info.usage = resource->info.buffer.flags;
					resource->info.buffer.buffer = m_Device->CreateBuffer(&info);
				}
			}
		}

		//create render passes, camera and framebuffers
		for (U32 i = 0; i < m_FrameGraph->m_SortedNodes.size(); ++i) {
			FrameGraphNode* node = &m_FrameGraph->GetNode(m_FrameGraph->m_SortedNodes[i]);
			node->renderPassControl->SetCamera(m_Camera);
			if (!HANDLE_IS_VALID((node->renderPass)))
			{
				CreateRenderPass(node);
				node->renderPassControl->SetNativeRenderPassHandle(node->renderPass);
			}

			if (!HANDLE_IS_VALID(node->frameBuffer)) {
				CreateFrameBuffer(node);
			}
		}

		//init render pass controls
		for (U32 i = 0; i < m_FrameGraph->m_SortedNodes.size(); ++i) {
			FrameGraphNode* node = &m_FrameGraph->GetNode(m_FrameGraph->m_SortedNodes[i]);
			if (node->renderPassControl != nullptr)
			{
				node->renderPassControl->SetFrameGraphData(m_FrameGraph, m_FrameGraph->m_SortedNodes[i]);
			}
			node->renderPassControl->SetAsTextureSource();
		}
	}

	FrameGraphNodeHandle FrameGraphBuilderHelper::CreateNode(FrameGraphNodeCreateData data)
	{
		FrameGraphNodeHandle nodeHandle{ Invalid_Handle_Id };
		nodeHandle = m_FrameGraph->CreateNewNode();

		FrameGraphNode* node = &m_FrameGraph->GetNode(nodeHandle);

		node->name = std::move(data.name);
		node->edges.reserve(data.outputs.size());
		node->inputs.reserve(data.inputs.size());
		node->outputs.reserve(data.outputs.size());
		node->frameBuffer = FrameBufferHandle{ Invalid_Handle_Id };
		node->renderPass = RenderPassHandle{ Invalid_Handle_Id };
		node->renderPassControl = data.control;
		node->clearColor = data.clearColor;
		node->supportResize = !data.ignoreResize;

		for (size_t i = 0; i < data.outputs.size(); ++i) {
			const FrameGraphResourceOutputCreation& output_creation = data.outputs[i];

			FrameGraphResourceHandle output = CreateOutputResource(output_creation, nodeHandle);
			node->outputs.push_back(output);
		}

		for (size_t i = 0; i < data.inputs.size(); ++i) {
			const FrameGraphResourceInputCreation& input_creation = data.inputs[i];

			FrameGraphResourceHandle input = CreateInputResource(input_creation, nodeHandle);
			node->inputs.push_back(input);
		}
		return nodeHandle;
	}

	FrameGraphResourceHandle FrameGraphBuilderHelper::CreateOutputResource(const FrameGraphResourceOutputCreation& output, FrameGraphNodeHandle node)
	{
		FrameGraphResourceHandle resourceHandle{ Invalid_Handle_Id };
		resourceHandle = m_FrameGraph->CreateNewResource();


		FrameGraphResource* resource = &m_FrameGraph->GetResource(resourceHandle);
		resource->name = output.name;
		resource->nameID = std::move(StringID::MakeStringID(output.name));
		resource->type = output.type;
		resource->resizable = !output.ignoreResize;

		if (output.type != FrameGraphResourceType::Reference) {
			resource->info = output.resource_info;
			resource->output_target = resourceHandle;
			resource->producer = node;
			resource->refCount = 0;
			m_FrameGraph->m_Resources.resourceMap[Hash(resource->name)].producerHandle = resourceHandle;
		}
		else {
			auto result = m_FrameGraph->m_Resources.resourceMap.find(Hash(output.name));
			if (result != m_FrameGraph->m_Resources.resourceMap.end()) {
				result->second.childProcessors.emplace_back(node);
			}
			else
			{
				FrameGraphProducerResourceInfo info;
				info.childProcessors.emplace_back(node);
				m_FrameGraph->m_Resources.resourceMap[Hash(output.name)] = std::move(info);
			}
		}

		return resourceHandle;
	}
	FrameGraphResourceHandle FrameGraphBuilderHelper::CreateInputResource(const FrameGraphResourceInputCreation& input, FrameGraphNodeHandle node)
	{
		FrameGraphResourceHandle resourceHandle{ Invalid_Handle_Id };
		resourceHandle = m_FrameGraph->CreateNewResource();

		FrameGraphResource* resource = &m_FrameGraph->GetResource(resourceHandle);
		resource->name = input.name;
		resource->nameID = std::move(StringID::MakeStringID(input.name));
		resource->type = input.type;

		return resourceHandle;
	}

	void FrameGraphBuilderHelper::ComputeEdgesFor(FrameGraphNode* node, Handle nodeIndex)
	{
		for (U32 r = 0; r < node->inputs.size(); ++r) {
			FrameGraphResource* resource = &m_FrameGraph->GetResource(node->inputs[r]);

			auto result = m_FrameGraph->m_Resources.resourceMap.find(Hash(resource->name));

			if (result == m_FrameGraph->m_Resources.resourceMap.end()) {
				continue;
			}

			FrameGraphResource* output_resource = &m_FrameGraph->GetResource(result->second.producerHandle);
			resource->producer = output_resource->producer;
			resource->info = output_resource->info;
			resource->output_target = output_resource->output_target;

			if (resource->type == FrameGraphResourceType::Reference || resource->type == FrameGraphResourceType::Texture)
			{
				if (result->second.HasAnyChildProcessor())
				{
					for (auto& processor : result->second.childProcessors)
					{
						FrameGraphNode* parent_node = &m_FrameGraph->GetNode(processor);
						parent_node->edges.push_back(FrameGraphNodeHandle{ nodeIndex });
					}
					continue; //if our input resource is a reference or texture we want to connect it to other references(we want to use it only after it was modified by all other nodes)
				}

			}

			FrameGraphNode* parent_node = &m_FrameGraph->GetNode(resource->producer);
			parent_node->edges.push_back(FrameGraphNodeHandle{ nodeIndex });

		}
	}

	void FrameGraphBuilderHelper::CreateRenderPass(FrameGraphNode* node)
	{
		RenderPassDesc renderPassDesc;
		for (U32 i = 0; i < node->outputs.size(); ++i) {
			FrameGraphResource* output_resource = &m_FrameGraph->GetResource(node->outputs[i]);

			FrameGraphResourceInfo& info = output_resource->info;

			if (output_resource->type == FrameGraphResourceType::Attachment) {
				if (IsDepthFormat(info.texture.format)) {
					renderPassDesc.SetDepthStencilTexture(info.texture.format, ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
					renderPassDesc.SetDepthAndStencilLoadOp(info.texture.loadOp, info.texture.loadOp);
				}
				else {
					renderPassDesc.AddAttachment(info.texture.format,ImageLayout::COLOR_ATTACHMENT_OPTIMAL, info.texture.loadOp);
				}
			}
		}

		for (U32 i = 0; i < node->inputs.size(); ++i) {
			FrameGraphResource* input_resource = &m_FrameGraph->GetResource(node->inputs[i]);

			FrameGraphResourceInfo& info = input_resource->info;

			if (input_resource->type == FrameGraphResourceType::Attachment) {
				if (IsDepthFormat(info.texture.format)) {
					renderPassDesc.SetDepthStencilTexture(info.texture.format,ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
					renderPassDesc.SetDepthAndStencilLoadOp(AttachmentLoadOp::LOAD, AttachmentLoadOp::LOAD);
				}
				else {
					renderPassDesc.AddAttachment(info.texture.format, ImageLayout::COLOR_ATTACHMENT_OPTIMAL, AttachmentLoadOp::LOAD);
				}
			}
		}

		renderPassDesc.type = node->renderPassControl->GetType();
		node->renderPass=m_Device->CreateRenderPass(&renderPassDesc);
	}

	void FrameGraphBuilderHelper::CreateFrameBuffer(FrameGraphNode* node)
	{
		FrameBufferDesc frameBufferDesc;
		frameBufferDesc.SetRenderPass(node->renderPass);
		U32 width = 0;
		U32 height = 0;

		for (U32 i = 0; i < node->outputs.size(); ++i) {
			FrameGraphResource* output_resource = &m_FrameGraph->GetResource(node->outputs[i]);

			FrameGraphResourceInfo& info = output_resource->info;

			if (width == 0 || height == 0)
			{
				width = info.texture.width;
				height = info.texture.height;
			}
			else if (width != info.texture.width || height != info.texture.height)
			{
				CZUCH_BE_ASSERT(false, "FrameBuffer attachments must have the same size");
			}

			if (output_resource->type == FrameGraphResourceType::Attachment) {
				if (IsDepthFormat(info.texture.format))
				{
					frameBufferDesc.SetDepthStencilTexture(info.texture.texture);
				}
				else
				{
					frameBufferDesc.AddRenderTexture(info.texture.texture);
				}
			}
		}

		for (U32 i = 0; i < node->inputs.size(); ++i) {
			FrameGraphResource* input_resource = &m_FrameGraph->GetResource(node->inputs[i]);

			if (input_resource->type != FrameGraphResourceType::Attachment)
			{
				continue;
			}

			FrameGraphResource* resource = &m_FrameGraph->GetResource(input_resource->output_target);

			FrameGraphResourceInfo& info = resource->info;

			if (width == 0 || height == 0)
			{
				width = info.texture.width;
				height = info.texture.height;
			}
			else if (width != info.texture.width || height != info.texture.height)
			{
				CZUCH_BE_ASSERT(false, "FrameBuffer attachments must have the same size");
			}

			if (input_resource->type == FrameGraphResourceType::Attachment) {
				if (IsDepthFormat(info.texture.format))
				{
					frameBufferDesc.SetDepthStencilTexture(info.texture.texture);
				}
				else
				{
					frameBufferDesc.AddRenderTexture(info.texture.texture);
				}
			}
		}

		frameBufferDesc.SetWidthAndHeight(width, height);
		node->frameBuffer = m_Device->CreateFrameBuffer(&frameBufferDesc);
	}
}