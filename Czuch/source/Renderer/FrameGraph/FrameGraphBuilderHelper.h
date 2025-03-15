#pragma once
#include"FrameGraph.h"

namespace Czuch
{

	struct FrameGraphResourceInputCreation {
		FrameGraphResourceType type;
		FrameGraphResourceInfo resource_info;

		const char* name;

		FrameGraphResourceInputCreation()
			: type(FrameGraphResourceType::Invalid), resource_info(), name(nullptr) {}
	};

	struct FrameGraphResourceOutputCreation {
		FrameGraphResourceType type;
		FrameGraphResourceInfo resource_info;

		const char* name;

		FrameGraphResourceOutputCreation()
			: type(FrameGraphResourceType::Invalid), resource_info(), name(nullptr) {}
	};

	struct FrameGraphNodeCreateData
	{
		Array<FrameGraphResourceInputCreation> inputs;
		Array<FrameGraphResourceOutputCreation> outputs;
		RenderPassControl* control;
		CzuchStr name;
		Vec3 clearColor;
		void Clear()
		{
			inputs.clear();
			outputs.clear();
			control = nullptr;
			clearColor = Vec3(0.0f);
		}
	};

	struct FrameGraphBuilderHelper
	{
		void Init(GraphicsDevice* device,Renderer* renderer);
		void Release();
		void BeginNewNode(const CzuchStr& name);
		void AddInput(FrameGraphResourceInputCreation input);
		void AddOutput(FrameGraphResourceOutputCreation output);
		void SetClearColor(const Vec3& color);
		void SetRenderPassControl(RenderPassControl* control);
		void EndNode();
		void Build(FrameGraph* graph);
	private:
		FrameGraphNodeHandle CreateNode(FrameGraphNodeCreateData data);
		FrameGraphResourceHandle CreateOutputResource(const FrameGraphResourceOutputCreation& output, FrameGraphNodeHandle node);
		FrameGraphResourceHandle CreateInputResource(const FrameGraphResourceInputCreation& input, FrameGraphNodeHandle node);
		void ComputeEdgesFor(FrameGraphNode* node, Handle nodeIndex);
		void CreateRenderPass(FrameGraphNode* node);
		void CreateFrameBuffer(FrameGraphNode* node);
	private:
		FrameGraphNodeCreateData m_CurrentNodeData;
		Array<FrameGraphNodeCreateData> m_NodesData;
		Array<FrameGraphNodeHandle> m_SortedNodes;
		Array<FrameGraphNodeHandle> tempNodes;
		bool m_Inited = false;
		GraphicsDevice* m_Device;
		FrameGraph *m_FrameGraph;
	};
}