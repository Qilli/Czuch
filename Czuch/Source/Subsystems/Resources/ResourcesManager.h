#pragma once
#include"Subsystems/BaseSubsystem.h"
#include"ResourceManager.h"
#include<vector>
#include<typeindex>

namespace Czuch
{
	class ResourcesManager: public BaseSubsystem<ResourcesManager>
	{
	public:
		void Init() override;
		void Shutdown() override;
		void Update() override;

		virtual void RegisterManager(ResourceManager* newMgr);

		template<class T>
		ResourceHandle LoadResource(const CzuchStr& path);

		template <typename T>
		void UnloadResource(ResourceHandle handle);

		template <typename T>
		T* GetResource(ResourceHandle handle);
		
	private:
		std::unordered_map<std::type_index,ResourceManager*> m_ResourcesMgrs;
	};

	template<class T>
	ResourceHandle ResourcesManager::LoadResource(const CzuchStr& path)
	{
		auto result = m_ResourcesMgrs.find(typeid(T));
		if (result != m_ResourcesMgrs.end())
		{
			auto mgr = result->second;
			Resource* res = mgr->LoadResource(path);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to load resource with path: {0} in manager of type {1}", path, result->first.name());
				return nullptr;
			}

			return res->GetHandle();
		}
		return { InvalidID };
	}

	template <typename T>
	T* ResourcesManager::GetResource(ResourceHandle handle)
	{
		auto result=m_ResourcesMgrs.find(typeid(T));
		if (result != m_ResourcesMgrs.end())
		{
			auto mgr = result->second;
			Resource* res=mgr->GetResource(handle);

			if (res == nullptr)
			{
				LOG_BE_ERROR("Failed to find resource with handle id: {0} in manager of type {1}", handle.id,result->first.name());
				return nullptr;
			}

			return static_cast<T*>(res);
		}
		LOG_BE_ERROR("Failed to find resource with handle id: {0}", handle.id);
		return nullptr;
	}
}

