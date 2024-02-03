#pragma once
#include"Core/EngineCore.h"
#include"Core/StringID.h"

namespace Czuch
{

	struct AssetHandle
	{
		Guid id;
	};

	enum AssetInnerState
	{
		CREATED,
		LOADED,
		UNLOADED
	};

	inline AssetHandle PathToHandle(const CzuchStr& path)
	{
		StringID id = sID(path);
		return {id.GetGuid()};
	}

	class CZUCH_API Asset
	{
	public:
		Asset(const CzuchStr& resourcePath, const CzuchStr& resourceName);
		virtual ~Asset() {}

		const CzuchStr& AssetPath() const { return m_AssetPath; }
		const CzuchStr& AssetName() const { return m_AssetName; }
		const Guid GetGuid() const { return m_GUID.GetGuid(); }

		inline bool IsLoaded() { return m_State; }
		inline AssetHandle GetHandle() { return {m_GUID.GetGuid()}; }

		static CzuchStr GetNameFromPath(const CzuchStr& inStr);
		static CzuchStr GetTypeFromPath(const CzuchStr& inStr);

		virtual bool LoadAsset() = 0;
		virtual void UnloadAsset() = 0;

	protected:
		AssetInnerState m_State;

	private:
		CzuchStr m_AssetPath;
		CzuchStr m_AssetName;
		StringID m_GUID;
	};

}

