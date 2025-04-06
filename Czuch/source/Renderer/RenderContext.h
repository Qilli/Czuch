#pragma once
#include"Graphics.h"
#include"Core/Math.h"

namespace Czuch
{
	class GraphicsDevice;
	struct TransformComponent;
	struct MeshComponent;
	struct MeshRendererComponent;
	struct LightComponent;
	struct Camera;

	struct RenderContextFillParams
	{
		MaterialInstanceHandle forcedMaterial;
		RenderPassType renderPassType;
		bool forceMaterialForAll;
		bool ignoreTransparent = false;
	};

	struct CZUCH_API RenderObjectInfo
	{
		TransformComponent* transform;
		MeshComponent* mesh;
		MeshRendererComponent* meshRenderer;
	};

	struct LightObjectInfo
	{
		LightData lightData;
		TransformComponent* transform;
		LightComponent* light;
	};

	struct CZUCH_API RenderObjectsContainer
	{
		Array<RenderObjectInfo> allObjects;
		Array<LightObjectInfo> allLights;
	};

	struct CZUCH_API RenderObjectInstance
	{
		Mat4x4 localToWorldTransformation;
		Mat4x4 localToClipSpaceTransformation;
		MeshHandle mesh;
		MaterialInstanceHandle overrideMaterial;
		int passIndex;

		RenderObjectInstance() = default;

		bool IsValid() const
		{
			return HANDLE_IS_VALID(mesh);
		}

		void UpdateSceneDataIfRequired(GraphicsDevice* device, SceneDataBuffers& sceneDataBuffers ,RenderContextFillParams& fillParams) const;

		RenderObjectInstance(RenderObjectInstance&& other) noexcept = default;
		RenderObjectInstance& operator=(RenderObjectInstance&& other) noexcept = default;

		RenderObjectInstance(const RenderObjectInstance& source) = default;
		RenderObjectInstance& operator=(const RenderObjectInstance&) = default;
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
		RenderContext(RenderContextCreateInfo& createInfo) :m_Settings(createInfo) { m_RenderObjects.reserve(1000); }
		RenderContext() = default;
		inline void ClearRenderList() { m_RenderObjects.clear(); }
		inline void AddToRenderList(RenderObjectInstance& instance) {m_RenderObjects.push_back(std::move(instance)); }
		inline const Array<RenderObjectInstance>& GetRenderObjectsList() const { return m_RenderObjects; }
		inline bool IsAutoCleanEnabled() const { return m_Settings.autoClearAfterRender; }
		inline RenderLayer GetRenderLayer() const { return m_Settings.renderLayer; }
		inline RenderType GetRenderType() const { return m_Settings.renderType; }
		inline int GetSortingOrder() const { return m_Settings.sortingOrder; }
		virtual void FillRenderList(GraphicsDevice* device, Camera* cam,RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams) = 0;
	protected:
		Array<RenderObjectInstance> m_RenderObjects;
		RenderContextCreateInfo m_Settings;
	};


	class CZUCH_API DefaultRenderContext : public RenderContext
	{
	public:
		DefaultRenderContext(RenderContextCreateInfo& createInfo) :RenderContext(createInfo) {}
		virtual void FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams) override;
	};

	class CZUCH_API DefaultTransparentRenderContext : public DefaultRenderContext
	{
	public:
		DefaultTransparentRenderContext(RenderContextCreateInfo& createInfo): DefaultRenderContext(createInfo) {}
		virtual void FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams) override;
	protected:
		void SortRenderObjects(Camera* cam);
	};

	class CZUCH_API DebugRenderContext : public RenderContext
	{
	public:
		DebugRenderContext(RenderContextCreateInfo& createInfo) :RenderContext(createInfo) {}
		virtual void FillRenderList(GraphicsDevice* device, Camera* cam, RenderObjectsContainer& allObjects, RenderContextFillParams& fillParams) override;
	};


	struct RenderContextContainer
	{
	public:
		void Add(RenderContext* ctx) { m_RenderContexts.push_back(ctx); Sort(); }
		void Remove(RenderContext* ctx) {
			int index = -1;
			for (int i = 0; i < m_RenderContexts.size(); i++)
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

