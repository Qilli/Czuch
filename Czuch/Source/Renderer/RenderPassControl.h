#pragma once
#include"Graphics.h"
#include"Core/EngineCore.h"
#include"RenderContext.h"

namespace Czuch
{

	class Renderer;
	class Camera;
	class CommandBuffer; 
	struct FrameGraph;

	class RenderPassControl
	{
	public:
		RenderPassControl(Camera* cam, int width, int height, RenderPassType type, bool handleWindowResize) : m_Width(width), m_Height(height), Type(type), m_HandleWindowResize(handleWindowResize), m_Camera(cam),m_Priority(10) {}
		virtual ~RenderPassControl() = default;
		virtual void PreDraw(CommandBuffer* cmd,Renderer* renderer);
		virtual void PostDraw(CommandBuffer* cmd,Renderer* renderer) = 0;
		virtual void Execute(CommandBuffer* cmd) = 0;
		virtual void* GetRenderPassResult() { return nullptr; }

		virtual void Resize(int width, int height);

		int GetWidth() const { return m_Width<=0?1:m_Width; }
		int GetHeight() const { return m_Height<=0?1:m_Height; }
		RenderPassType GetType() const { return Type; }
		int GetPriority() const { return m_Priority; }
		void SetPriority(int priority) { m_Priority = priority; }
		bool IsDifferentAspect(int width, int height) const 
		{ 
			if(height==0)
			{
				return true;
			}
			float currentAspect = GetCurrentAspect();
			float newAspect = width / (float)height;
			return abs(currentAspect - newAspect) > EPSILON;
		}
		bool IsDifferentCamera(Camera* cam) const { return GetCamera() != cam; }
		virtual Camera* GetCamera() const { return m_Camera; }
		bool HandleWindowResize() const { return m_HandleWindowResize; }
		float GetCurrentAspect() const { return m_Width / (float)m_Height; }
		void SetFrameGraphData(FrameGraph* fgraph, FrameGraphNodeHandle node) { m_FrameGraph = fgraph; m_Node = node; }//todo it should be exectued when building framegraph
	protected:
		int m_Width, m_Height;
		int m_LastWidth, m_LastHeight;

		int m_Priority;
		bool m_HandleWindowResize;
		Camera* m_Camera;
		Camera* m_LastCamera;
		RenderPassType Type;
		FrameGraphNodeHandle m_Node;
		FrameGraph* m_FrameGraph;
	};
}
