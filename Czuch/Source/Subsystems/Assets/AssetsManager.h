#pragma once
#include"Subsystems/BaseSubsystem.h"
#include"AssetManager.h"
#include"SettingsPerType.h"
#include<vector>
#include<typeindex>


namespace Czuch
{
	static const CzuchStr ASSETS_PATH = "F:/Engine/Czuch/Czuch/Data";

	class CZUCH_API AssetsManager: public BaseSubsystem<AssetsManager>
	{
	public:
		void Init(RenderSettings* settings) override;
		void Shutdown() override;
		void Update(TimeDelta timeDelta) override;

		virtual void RegisterManager(AssetManager* newMgr,std::type_index type);

		template<class T,class TM>
		AssetHandle LoadAsset(const CzuchStr& path,TM&& settings);

		template<class T, class TM>
		AssetHandle LoadAsset(const CzuchStr& path, TM& settings);

		template<class T,class TM>
		AssetHandle CreateAsset(const CzuchStr& name, TM&& createSettings);

		template<class T, class TM>
		AssetHandle CreateAsset(const CzuchStr& name, TM& createSettings);

		template<class T>
		AssetHandle LoadAsset(const CzuchStr& path);

		template <typename T>
		void UnloadAsset(AssetHandle handle);

		template <typename T>
		T* GetAsset(AssetHandle handle);

		template<typename T,typename TM>
		T LoadAndGetResouceHandle(const CzuchStr& path, TM&& settings);
	public:
		TextureHandle Load2DTexture(const CzuchStr& path);
		MaterialInstanceHandle CreateMaterialInstance(MaterialInstanceCreateSettings& settings);
		MaterialInstanceHandle CreateMaterialInstance(const CzuchStr& matName, AssetHandle materialSource);
	private:
		std::unordered_map<std::type_index,AssetManager*> m_AssetsMgrs;
	};

	template<class T, class TM>
	AssetHandle AssetsManager::LoadAsset(const CzuchStr& path, TM&& settings)
	{
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			Asset* res = mgr->LoadAsset(ASSETS_PATH+path,settings);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to load resource with path: {0} in manager of type {1}", path, result->first.name());
				return {InvalidID};
			}

			return res->GetHandle();
		}
		return { InvalidID };
	}

	template<class T, class TM>
	AssetHandle AssetsManager::LoadAsset(const CzuchStr& path, TM& settings)
	{
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			Asset* res = mgr->LoadAsset(ASSETS_PATH+path, settings);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to load resource with path: {0} in manager of type {1}", path, result->first.name());
				return { InvalidID };
			}

			return res->GetHandle();
		}
		return { InvalidID };
	}

	template<class T, class TM>
	inline AssetHandle AssetsManager::CreateAsset(const CzuchStr& name, TM&& createSettings)
	{
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			Asset* res = mgr->CreateAsset(name,createSettings);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to create asset with name: {0} in manager of type {1}", name, result->first.name());
				return { InvalidID };
			}

			return res->GetHandle();
		}
		return { InvalidID };
	}

	template<class T, class TM>
	inline AssetHandle AssetsManager::CreateAsset(const CzuchStr& name, TM& createSettings)
	{
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			Asset* res = mgr->CreateAsset(name, createSettings);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to create asset with name: {0} in manager of type {1}", name, result->first.name());
				return { InvalidID };
			}

			return res->GetHandle();
		}
		LOG_BE_ERROR("Failed to find manager for asset with name {0}",name);
		return { InvalidID };
	}


	template<class T>
	AssetHandle AssetsManager::LoadAsset(const CzuchStr& path)
	{
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			BaseLoadSettings defaultSettings;
			Asset* res = mgr->LoadAsset(ASSETS_PATH+path,defaultSettings);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to load asset with path: {0} in manager of type {1}", path, result->first.name());
				return { InvalidID };
			}

			return res->GetHandle();
		}
		return { InvalidID };
	}

	template<typename T>
	inline void AssetsManager::UnloadAsset(AssetHandle handle)
	{
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			mgr->UnloadAsset(handle);
			
		}
		LOG_BE_ERROR("Failed to find asset for removal with handle id: {0}", handle.handle);
	}




	template <typename T>
	T* AssetsManager::GetAsset(AssetHandle handle)
	{
		auto result=m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			Asset* res=mgr->GetAsset(handle);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to find asset with handle id: {0} in manager of type {1}", handle.handle,result->first.name());
				return nullptr;
			}

			return static_cast<T*>(res);
		}
		LOG_BE_ERROR("Failed to find asset with handle id: {0}", handle.handle);
		return nullptr;
	}
	template<typename T, typename TM>
	T AssetsManager::LoadAndGetResouceHandle(const CzuchStr& path, TM&& settings)
	{
		AssetHandle asset = LoadAsset<T>(path, settings);
		auto assetObj = GetAsset<T>(asset);
		return assetObj;
	}
}

