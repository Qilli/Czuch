#pragma once
#include"./Renderer/Graphics.h"

namespace Czuch
{
	class CommandBuffer;
	class RenderPassControl;
	class Renderer;

	struct FrameGraphResourceHandle
	{
		Handle handle;
	};

	struct FrameGraphProducerResourceInfo
	{
		FrameGraphResourceHandle producerHandle;
		Array<FrameGraphNodeHandle> childProcessors;

		FrameGraphProducerResourceInfo() : producerHandle({ 0 }), childProcessors(10) {}
		bool HasAnyChildProcessor() const { return childProcessors.size() > 0; }
	};

	enum class FrameGraphResourceType {
		Invalid = -1,
		Buffer = 0,
		Texture = 1,
		Attachment = 2,
		Reference = 3
	};

	struct FrameGraphResourceInfo {
		bool external = false;
		union {
			struct {
				BufferHandle buffer;
				size_t size;
				Usage flags;
			} buffer;

			struct {
				AttachmentLoadOp loadOp;
				U32 width, height, depth;
				ImageUsageFlag usage;
				Format format;
				TextureHandle texture;
			} texture;
		};

		FrameGraphResourceInfo() : external(false) {}
	};

	struct FrameGraphResource
	{
		FrameGraphResourceType type;
		FrameGraphResourceInfo info;
		FrameGraphNodeHandle producer;
		FrameGraphResourceHandle output_target;
		U32 refCount = 0;
		CzuchStr name;
	};

	struct FrameGraph;
	struct FrameGraphNode
	{
		RenderPassHandle renderPass;
		FrameBufferHandle frameBuffer;
		RenderPassControl* renderPassControl;
		Array<FrameGraphResourceHandle> inputs;
		Array<FrameGraphResourceHandle> outputs;
		Array<FrameGraphNodeHandle> edges;
		CzuchStr name;

		void Release(GraphicsDevice* device);
		void Resize(GraphicsDevice* device, U32 width, U32 height);
		TextureHandle GetFirstColorAttachment(FrameGraph* fgraph);
	};

	struct FrameGraphNodesContainer {
		void Init(GraphicsDevice* device);
		void Release();
		FrameGraphNode& GetNode(FrameGraphNodeHandle handle);
		FrameGraphNodeHandle CreateNewNode();
		Array<FrameGraphNode> m_Nodes;
		GraphicsDevice* device;
	};

	struct FrameGraphResourcesContainer {
		void Init(GraphicsDevice* device);
		void Release();
		FrameGraphResourceHandle CreateNewResource();
		FrameGraphResource& GetResource(FrameGraphResourceHandle handle);
		Array<FrameGraphResource> resources;
		std::unordered_map<U32, FrameGraphProducerResourceInfo> resourceMap;
	};

	struct FrameGraph
	{
		void Init(GraphicsDevice* device,Renderer* renderer);
		void Release();
		FrameGraphNode& GetNode(FrameGraphNodeHandle handle) { return m_Nodes.GetNode(handle); }
		FrameGraphResource& GetResource(FrameGraphResourceHandle handle) { return m_Resources.GetResource(handle); }
		FrameGraphNodeHandle CreateNewNode();
		FrameGraphResourceHandle CreateNewResource();
		void Execute(GraphicsDevice* device,CommandBuffer* cmd);
		void ResizeNode(FrameGraphNode node, U32 width, U32 height);
		void* GetRenderPassResult(RenderPassType type);
		RenderPassControl* GetRenderPassControlByType(RenderPassType type);
		bool HasRenderPass(RenderPassType type);
		void ResizeRenderPasses(U32 width, U32 height, bool windowSizeChanged = true);
		TextureHandle GetFinalTexture();
		bool HasUI() { return m_HasUI; }
		void AddUI() { m_HasUI = true; }
		//[TODO] we need to add option to init offscreen node with size from UI
	private:

	private:
		FrameGraphNodesContainer m_Nodes;
		FrameGraphResourcesContainer m_Resources;
		Array<FrameGraphNodeHandle> m_SortedNodes;
		GraphicsDevice* m_Device;
		Renderer* m_Renderer;
		bool m_HasUI = false;
		friend class FrameGraphBuilderHelper;
	};
}
