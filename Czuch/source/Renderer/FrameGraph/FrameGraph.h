#pragma once
#include"./Renderer/Graphics.h"

namespace Czuch
{
	class CommandBuffer;
	class RenderPassControl;
	class Renderer;
	class GraphicsDevice;
	struct Camera;

	struct FrameGraphResourceHandle
	{
		Handle handle;
	};

	struct FrameGraphProducerResourceInfo
	{
		FrameGraphResourceHandle producerHandle;
		Array<FrameGraphNodeHandle> childProcessors;

		FrameGraphProducerResourceInfo() : producerHandle({ 0 }), childProcessors() {}
		bool HasAnyChildProcessor() const { return childProcessors.size() > 0; }
	};

	enum class FrameGraphResourceType {
		Invalid = -1,
		Buffer = 0,
		Texture = 1,
		Attachment = 2,
		Reference = 3
	};

	struct FrameGrapBufferResource
	{
		BufferHandle buffer;
		size_t size;
		Usage flags;
	};

	struct FrameGraphTextureResource {
		AttachmentLoadOp loadOp;
		U32 width, height, depth;
		ImageUsageFlag usage;
		Format format;
		TextureHandle texture;

		FrameGraphTextureResource() : loadOp(AttachmentLoadOp::CLEAR), width(0), height(0), depth(0), usage(ImageUsageFlag::COLOR_ATTACHMENT), format(Format::R8G8B8A8_UNORM), texture({ InvalidID })
		{
			
		}
	};

	struct FrameGraphResourceInfo {
		bool external = false;
		FrameGrapBufferResource buffer;
		FrameGraphTextureResource texture;

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
		StringID nameID;
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
		Vec3 clearColor=Vec3(0,0,0);
		CzuchStr name;

		void Release(GraphicsDevice* device);
		void Resize(GraphicsDevice* device, U32 width, U32 height);
		TextureHandle GetFirstColorAttachment(FrameGraph* fgraph);
		TextureHandle GetDepthAttachment(FrameGraph* fgraph);
	};

	struct FrameGraphNodesContainer {
		void Init(GraphicsDevice* device);
		void Release();
		void ReleaseDependencies();
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


	struct CZUCH_API FrameGraph
	{
		void Init(Camera* camera,GraphicsDevice* device,Renderer* renderer);
		void Release();
		FrameGraphNode& GetNode(FrameGraphNodeHandle handle) { return m_Nodes.GetNode(handle); }
		FrameGraphResource& GetResource(FrameGraphResourceHandle handle) { return m_Resources.GetResource(handle); }
		FrameGraphNodeHandle CreateNewNode();
		FrameGraphResourceHandle CreateNewResource();
		void AfterSystemInit();
		void Execute(GraphicsDevice* device,CommandBuffer* cmd);
		void ResizeNode(FrameGraphNode node, U32 width, U32 height);
		Camera* GetCamera() { return m_Camera; }
		void* GetRenderPassResult(RenderPassType type);
		void* GetFinalRenderPassResult();
		RenderPassControl* GetRenderPassControlByType(RenderPassType type);
		RenderPassHandle GetFinalRenderPassHandle();
		bool HasRenderPass(RenderPassType type);
		void ResizeRenderPasses(U32 width, U32 height, bool windowSizeChanged = true);
		TextureHandle GetFinalTexture();
		TextureHandle GetFinalDepthTexture();
		bool HasUI() { return m_HasUI; }
		void AddUI() { m_HasUI = true; }
		U32 GetNodesCount() { return m_Nodes.m_Nodes.size(); }
		U32 GetResourceCount() { return m_Resources.resources.size(); }
		void* GetRenderPassResultAt(U32 renderPassIndex);
		CzuchStr& GetNodeName(FrameGraphNodeHandle handle) { return m_Nodes.GetNode(handle).name; }
		CzuchStr& GetNodeNameAt(U32 index) { return m_Nodes.m_Nodes[index].name; }
		void SetClearColor(Color color) { m_ClearColor = color; m_UseClearColor = true; }
		Color GetClearColor() { return m_ClearColor; }
		void SetUseClearColor(bool use) { m_UseClearColor = use; }
		/// <summary>
		/// called before rendering another frame, here we fill and create required contexts
		/// </summary>
		void BeforeFrameGraphExecute(CommandBuffer* cmd);
		/// <summary>
		/// Called after frame was rendered, here we can invalidate or make dirty current contexts
		/// </summary>
		/// <param name="cmd"></param>
		void AfterFrameGraphExecute(CommandBuffer* cmd);
		//[TODO] we need to add option to init offscreen node with size from UI

		//Set debug render flag, depending on the flag
		//it may require different action, for instance for material index debug flag
		//we change lighting main shader to debug version where each pixel has a different color depending on the used material
		void SetDebugRenderFlag(DebugRenderingFlag flag,bool enable);
	private:
		void ReleaseDependencies();
	private:
		FrameGraphNodesContainer m_Nodes;
		FrameGraphResourcesContainer m_Resources;
		Array<FrameGraphNodeHandle> m_SortedNodes;
		Camera* m_Camera;
		GraphicsDevice* m_Device;
		Renderer* m_Renderer;
		Color m_ClearColor;
		bool m_HasUI = false;
		bool m_UseClearColor = false;
		friend class FrameGraphBuilderHelper;
	};
}
