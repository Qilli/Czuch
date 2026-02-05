#pragma once
#include "RendererCommon.h"

namespace Czuch
{
	class CZUCH_API RenderContext
	{
	public:
		RenderContext(RenderContextCreateInfo &createInfo) : m_Settings(createInfo)
		{
			m_RenderObjects.reserve(1000);
			m_IsDirty = true;
		}
		RenderContext() = default;
		inline void ClearRenderList()
		{
			m_RenderObjects.clear();
			m_IsDirty = true;
		}
		inline void AddToRenderList(RenderObjectInstance &instance) { m_RenderObjects.push_back(std::move(instance)); }
		inline const Array<RenderObjectInstance> &GetRenderObjectsList() const { return m_RenderObjects; }
		inline bool IsAutoCleanEnabled() const { return m_Settings.autoClearBeforeRender; }
		inline RenderLayer GetRenderLayer() const { return m_Settings.renderLayer; }
		inline RenderType GetRenderType() const { return m_Settings.renderType; }
		inline int GetSortingOrder() const { return m_Settings.sortingOrder; }
		virtual void FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams) = 0;
		inline bool IsDirty() const { return m_IsDirty; }
		virtual bool SupportRenderPass(RenderPassType type) const;

	protected:
		Array<RenderObjectInstance> m_RenderObjects;
		RenderContextCreateInfo m_Settings;
		bool m_IsDirty = false;
	};

	class CZUCH_API DefaultRenderContext : public RenderContext
	{
	public:
		DefaultRenderContext(RenderContextCreateInfo &createInfo) : RenderContext(createInfo) {}
		virtual void FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams) override;
		bool SupportRenderPass(RenderPassType type) const override;

	private:
		const U32 SUPPORTED_RENDER_PASSES_FLAGS =
			(U32)RenderPassType::ForwardLighting;
	};

	class CZUCH_API DefaultDepthPrePassRenderContext : public DefaultRenderContext
	{
	public:
		DefaultDepthPrePassRenderContext(RenderContextCreateInfo &createInfo) : DefaultRenderContext(createInfo) {}
		void FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams) override;
		bool SupportRenderPass(RenderPassType type) const override;

	private:
		const U32 SUPPORTED_RENDER_PASSES_FLAGS = (U32)RenderPassType::DepthPrePass | (U32)RenderPassType::DepthLinearPrePass | (U32)RenderPassType::DepthLinearShadowMapPrePass;
	};

	class CZUCH_API DefaultDirectionalShadowMapRenderContext : public DefaultRenderContext
	{
	public:
		DefaultDirectionalShadowMapRenderContext(RenderContextCreateInfo &createInfo) : DefaultRenderContext(createInfo) {}
		void FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams) override;
		bool SupportRenderPass(RenderPassType type) const override;

	private:
		const U32 SUPPORTED_RENDER_PASSES_FLAGS = (U32)RenderPassType::DirectionalShadowMap;
	};

	class CZUCH_API DefaultTransparentRenderContext : public DefaultRenderContext
	{
	public:
		DefaultTransparentRenderContext(RenderContextCreateInfo &createInfo) : DefaultRenderContext(createInfo) {}
		virtual void FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams) override;
		bool SupportRenderPass(RenderPassType type) const override;

	protected:
		void SortRenderObjects(Camera *cam);

	private:
		const U32 SUPPORTED_RENDER_PASSES_FLAGS =
			(U32)RenderPassType::ForwardLightingTransparent;
	};

	class CZUCH_API DebugRenderContext : public RenderContext
	{
	public:
		DebugRenderContext(RenderContextCreateInfo &createInfo) : RenderContext(createInfo) {}
		virtual void FillRenderList(GraphicsDevice *device, Camera *cam, RenderObjectsContainer &allObjects, RenderContextFillParams &fillParams) override;
		bool SupportRenderPass(RenderPassType type) const override;

	private:
		const U32 SUPPORTED_RENDER_PASSES_FLAGS =
			(U32)RenderPassType::DebugDraw;
	};

}
