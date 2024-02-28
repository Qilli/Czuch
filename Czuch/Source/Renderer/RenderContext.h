#pragma once
#include"Graphics.h"
#include"Core/Math.h"

namespace Czuch
{
	class GraphicsDevice;

	struct RenderObjectInstance
	{
		MeshHandle mesh;
		MaterialHandle overrideMaterial;
		Mat4x4 localToWorldTransformation;
		Mat4x4 localToClipSpaceTransformation;

		bool IsValid() const
		{
			return HANDLE_IS_VALID(mesh);
		}

		void UpdateSceneDataIfRequired(GraphicsDevice* device,BufferHandle buffer);
	};

	struct RenderContextCreateInfo
	{
		RenderLayer renderLayer;
		RenderType renderType;
		bool autoClearAfterRender;
	};


	class RenderContext
	{
	public:
		RenderContext(RenderContextCreateInfo& createInfo) :m_Settings(createInfo) {}
		inline void ClearRenderList() { m_RenderObjects.clear(); }
		inline void AddToRenderList(RenderObjectInstance& instance) { m_RenderObjects.push_back(std::move(instance)); }
		inline const std::vector<RenderObjectInstance>& GetRenderObjectsList() const { return m_RenderObjects; }
		inline bool IsAutoCleanEnabled() const { return m_Settings.autoClearAfterRender; }
	private:
		std::vector<RenderObjectInstance> m_RenderObjects;
		RenderContextCreateInfo m_Settings;
	};

	struct RenderContextContainer
	{
	public:
		void Add(RenderContext* ctx) { m_RenderContexts.push_back(ctx); }
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
		std::vector<RenderContext*> m_RenderContexts;
	};

}

