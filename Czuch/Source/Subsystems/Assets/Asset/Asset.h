#pragma once
#include"Core/EngineCore.h"
#include"Core/StringID.h"

namespace Czuch
{
	class AssetsManager;
	struct AssetHandle
	{
		Guid handle;
	};

	enum AssetInnerState
	{
		CREATED,
		LOADED,
		UNLOADED
	};

	enum AssetType
	{
		LOADED_TYPE,
		CREATED_TYPE
	};

	inline AssetHandle PathToHandle(const CzuchStr& path)
	{
		StringID id = sID(path);
		return {id.GetGuid()};
	}
	struct AssetRefCounter
	{
		int counter = 0;
		void Up()
		{
			counter++;
		}

		bool Down()
		{
			counter--;
			if (counter < 0)
			{
				counter = 0;
			}
			return counter > 0;
		}
	};

	class CZUCH_API Asset
	{
	public:
		Asset(const CzuchStr& resourcePath, const CzuchStr& resourceName,AssetsManager* assetsManager);
		Asset(const CzuchStr& resourceName, AssetsManager* assetsManager);
		virtual ~Asset() {}

		const CzuchStr& AssetPath() const { return m_AssetPath; }
		const CzuchStr& AssetName() const { return m_AssetName; }
		const Guid GetGuid() const { return m_GUID.GetGuid(); }

		inline bool IsLoaded() { return m_State; }
		inline AssetHandle GetHandle() { return {m_GUID.GetGuid()}; }

		static CzuchStr GetNameFromPath(const CzuchStr& inStr);
		static CzuchStr GetTypeFromPath(const CzuchStr& inStr);

		virtual bool LoadAsset() = 0;
		virtual bool UnloadAsset() = 0;
		virtual bool CreateFromData() = 0;
	protected:
		AssetInnerState m_State;
		AssetType m_AssetType;
		AssetRefCounter m_RefCounter;
		bool m_ForceUnload;
		AssetsManager* m_AssetsMgr;
	private:
		CzuchStr m_AssetPath;
		CzuchStr m_AssetName;
		StringID m_GUID;
	};

}

