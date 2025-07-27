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
	struct FrameGraphNode;

	struct MaterialsForInputBinding
	{
		Array<MaterialInstanceHandle> m_Materials;
		void AddMaterial(MaterialInstanceHandle material) { m_Materials.push_back(material); }
		void Clear() { m_Materials.clear(); }
		void BindTextureForMaterials(GraphicsDevice* device,StringID& texName,TextureHandle texture);
	};

	class RenderPassControl
	{
	public:
		RenderPassControl(Camera* cam, int width, int height, RenderPassType type, bool handleWindowResize, bool uiTextureSource = false);
		virtual ~RenderPassControl() = default;
		virtual void Init() = 0;
		virtual void BindInputTextures(GraphicsDevice* device, FrameGraphNode* node) = 0;
		virtual void PreDraw(CommandBuffer* cmd,Renderer* renderer);
		virtual void PostDraw(CommandBuffer* cmd,Renderer* renderer);
		virtual void BeforeFrameGraphExecute(CommandBuffer* cmd, Renderer* renderer);
		virtual void AfterFrameGraphExecute(CommandBuffer* cmd, Renderer* renderer);
		virtual void SetCamera(Camera* cam) { m_Camera = cam; }
		virtual void Execute(CommandBuffer* cmd) = 0;
		virtual void* GetRenderPassResult() { return nullptr; }
		virtual void ReleaseDependencies() = 0;
		virtual void TransitionResultsToShaderReadOnly(CommandBuffer* cmd) = 0;


		virtual void Resize(int width, int height);
		virtual void Release()=0;
		virtual void SetAsTextureSource() = 0;

		void SetActive(bool active) { m_Active = active; }
		bool IsActive() const { return m_Active; }

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
		void SetFrameGraphData(FrameGraph* fgraph, FrameGraphNodeHandle node) { m_FrameGraph = fgraph; m_Node = node; }
		FrameGraphNodeHandle GetNode() const { return m_Node; }
		FrameGraph* GetFrameGraph() const { return m_FrameGraph; }
		void SetNativeRenderPassHandle(RenderPassHandle handle) { m_NativeRenderPassHandle = handle; }
		RenderPassHandle GetNativeRenderPassHandle() const { return m_NativeRenderPassHandle; }
		void AddMaterialForInputBinding(MaterialInstanceHandle material);
		void ForceSingleMaterialForAll(MaterialInstanceHandle material,bool ignoreTransparent);
		void ClearForceSingleMaterialForAll();
	protected:
		MaterialsForInputBinding m_MaterialsForInputBinding;
		RenderContextFillParams m_FillParams;
		int m_Width, m_Height;
		int m_LastWidth, m_LastHeight;
		int m_Priority;
		bool m_HandleWindowResize;
		bool m_UITextureSource;
		bool m_Active;
		Camera* m_Camera;
		Camera* m_LastCamera;
		RenderPassType Type;
		RenderPassHandle m_NativeRenderPassHandle;
		FrameGraphNodeHandle m_Node;
		FrameGraph* m_FrameGraph;
	};
}
