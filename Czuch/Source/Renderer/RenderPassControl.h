#pragma once
#include"Graphics.h"

namespace Czuch
{
	enum class CZUCH_API RenderPassType
	{
		MainForward,
		Shadow,
		PostProcess,
		OffscreenTexture,
		Custom
	};

	class Renderer;
	class Camera;
	class CommandBuffer;
	class RenderPassControl
	{
	public:
		RenderPassControl(Camera* cam, int width, int height, RenderPassType type, bool handleWindowResize) : m_Width(width), m_Height(height), Type(type), m_HandleWindowResize(handleWindowResize), m_Camera(cam),m_Priority(10) {}
		virtual ~RenderPassControl() = default;
		virtual void BeginRenderPass(CommandBuffer* cmd) = 0;
		virtual void EndRenderPass(CommandBuffer* cmd) = 0;
		virtual void Execute(CommandBuffer* cmd) = 0;
		virtual void* GetRenderPassResult() { return nullptr; }

		virtual void Resize(int width, int height) { m_Width = width; m_Height = height; }

		int GetWidth() const { return m_Width; }
		int GetHeight() const { return m_Height; }
		RenderPassType GetType() const { return Type; }
		int GetPriority() const { return m_Priority; }
		void SetPriority(int priority) { m_Priority = priority; }
		bool IsDifferentAspect(int width, int height) const { return ((m_Width / (float)m_Height) - (width / (float)height)) > EPSILON; }
		bool IsDifferentCamera(Camera* cam) const { return m_Camera != cam; }
		Camera* GetCamera() const { return m_Camera; }
		bool HandleWindowResize() const { return m_HandleWindowResize; }
	protected:
		int m_Width, m_Height;
		int m_Priority;
		bool m_HandleWindowResize;
		Camera* m_Camera;
		RenderPassType Type;
	};
}
