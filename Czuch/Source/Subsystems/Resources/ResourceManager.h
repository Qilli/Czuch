#pragma once
#include"Core/EngineCore.h"
#include"Resource.h"
#include<unordered_map>

namespace Czuch
{
	class ResourceManager
	{
	public:
		ResourceManager();
		virtual ~ResourceManager();
		virtual Resource* LoadResource(const CzuchStr& path);
		virtual Resource* LoadResource(ResourceHandle handle);
		virtual void UnloadResource(ResourceHandle handle);
		virtual void UnloadResource(const CzuchStr& path);
		virtual void UnloadAll();
		virtual ResourceHandle GetHandleForResource(const CzuchStr& path);
		virtual Resource* GetResource(ResourceHandle handle);
	protected:
		virtual Resource* CreateResource(const CzuchStr& path)=0;
	private:
		std::unordered_map<Guid, Resource*> m_Resources;
	};
}

