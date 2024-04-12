#pragma once
#include"Graphics.h"
#include"Core/Math.h"

namespace Czuch
{
	class GraphicsDevice;

	struct CZUCH_API RenderObjectInstance
	{
		MeshHandle mesh;
		MaterialInstanceHandle overrideMaterial;
		Mat4x4 localToWorldTransformation;
		Mat4x4 localToClipSpaceTransformation;

		bool IsValid() const
		{
			return HANDLE_IS_VALID(mesh);
		}

		void UpdateSceneDataIfRequired(GraphicsDevice* device,BufferHandle buffer);
	};

	struct CZUCH_API RenderContextCreateInfo
	{
		RenderLayer renderLayer;
		RenderType renderType;
		int sortingOrder;
		bool autoClearAfterRender;

		RenderContextCreateInfo()
		{
			renderLayer = RenderLayer::LAYER_0;
			renderType = RenderType::General;
			sortingOrder = 0;
			autoClearAfterRender = true;
		}
	};


	class CZUCH_API RenderContext
	{
	public:
		RenderContext(RenderContextCreateInfo& createInfo) :m_Settings(createInfo) {}
		RenderContext() = default;
		inline void ClearRenderList() { m_RenderObjects.clear(); }
		inline void AddToRenderList(RenderObjectInstance instance) { m_RenderObjects.push_back(std::move(instance)); }
		inline const std::vector<RenderObjectInstance>& GetRenderObjectsList() const { return m_RenderObjects; }
		inline bool IsAutoCleanEnabled() const { return m_Settings.autoClearAfterRender; }
		inline RenderLayer GetRenderLayer() const { return m_Settings.renderLayer; }
		inline RenderType GetRenderType() const { return m_Settings.renderType; }
		inline int GetSortingOrder() const { return m_Settings.sortingOrder; }
	private:
		std::vector<RenderObjectInstance> m_RenderObjects;
		RenderContextCreateInfo m_Settings;
	};

	struct RenderContextContainer
	{
	public:
		void Add(RenderContext* ctx) { m_RenderContexts.push_back(ctx); Sort(); }
		void Remove(RenderContext* ctx) {
			int index = -1;
			for (size_t i = 0; i < m_RenderContexts.size(); i++)
			{
				if (m_RenderContexts[i] == ctx) {
					index = i;
					break;
				}
			}
			if (index >= 0)
			{
				m_RenderContexts.erase(m_RenderContexts.begin()+index);
			}
		}
		void Sort()
		{
			std::sort(m_RenderContexts.begin(), m_RenderContexts.end(), [](RenderContext* a, RenderContext* b) {
				if (a->GetRenderType() == b->GetRenderType())
				{
				     return a->GetSortingOrder() < b->GetSortingOrder();
			    }

				return a->GetRenderType() < b->GetRenderType();
				});
		}
		std::vector<RenderContext*> m_RenderContexts;
	};

}

