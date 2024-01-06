#pragma once
#include"Core/EngineCore.h"
#include"Core/StringID.h"

namespace Czuch
{

	struct ResourceHandle
	{
		Guid id;
	};

	enum ResourceInnerState
	{
		CREATED,
		LOADED,
		UNLOADED
	};

	inline ResourceHandle PathToHandle(const CzuchStr& path)
	{
		StringID id = sID(path);
		return {id.GetGuid()};
	}

	class CZUCH_API Resource
	{
	public:
		Resource(const CzuchStr& resourcePath, const CzuchStr& resourceName);

		const CzuchStr& ResourcePath() const { return m_ResourcePath; }
		const CzuchStr& ResourceName() const { return m_ResourceName; }
		const Guid GetGuid() const { return m_GUID.GetGuid(); }

		inline bool IsLoaded() { return m_State; }
		inline ResourceHandle GetHandle() { return {m_GUID.GetGuid()}; }

		static CzuchStr& GetNameFromPath(const CzuchStr& inStr);
		static CzuchStr& GetTypeFromPath(const CzuchStr& inStr);

		virtual bool LoadResource() = 0;
		virtual void UnloadResource() = 0;

	protected:
		ResourceInnerState m_State;

	private:
		CzuchStr m_ResourcePath;
		CzuchStr m_ResourceName;
		StringID m_GUID;
	};

}

