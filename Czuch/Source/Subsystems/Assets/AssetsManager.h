#pragma once
#include"Subsystems/BaseSubsystem.h"
#include"AssetManager.h"
#include"LoadSettingsPerType.h"
#include<vector>
#include<typeindex>


namespace Czuch
{
	class AssetsManager: public BaseSubsystem<AssetsManager>
	{
	public:
		void Init() override;
		void Shutdown() override;
		void Update() override;

		virtual void RegisterManager(AssetManager* newMgr,std::type_index type);

		template<class T,class TM>
		AssetHandle LoadAsset(const CzuchStr& path,TM settings);

		template<class T>
		AssetHandle LoadAsset(const CzuchStr& path);

		template <typename T>
		void UnloadAsset(AssetHandle handle);

		template <typename T>
		T* GetAsset(AssetHandle handle);
		
	private:
		std::unordered_map<std::type_index,AssetManager*> m_AssetsMgrs;
	};

	template<class T, class TM>
	AssetHandle AssetsManager::LoadAsset(const CzuchStr& path, TM settings)
	{
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			mgr->SetSettings(static_cast<void*>(&settings));
			Asset* res = mgr->LoadAsset(path);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to load resource with path: {0} in manager of type {1}", path, result->first.name());
				return {InvalidID};
			}

			return res->GetHandle();
		}
		return { InvalidID };
	}


	template<class T>
	AssetHandle AssetsManager::LoadAsset(const CzuchStr& path)
	{
		auto result = m_AssetsMgrs.find(typeid(T));
		if (result != m_AssetsMgrs.end())
		{
			auto mgr = result->second;
			mgr->SetSettings(nullptr);
			Asset* res = mgr->LoadAsset(path);

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
		LOG_BE_ERROR("Failed to find asset for removal with handle id: {0}", handle.id);
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
				LOG_BE_ERROR("Failed to find asset with handle id: {0} in manager of type {1}", handle.id,result->first.name());
				return nullptr;
			}

			return static_cast<T*>(res);
		}
		LOG_BE_ERROR("Failed to find asset with handle id: {0}", handle.id);
		return nullptr;
	}
}

