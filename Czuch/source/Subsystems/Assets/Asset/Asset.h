#pragma once
#include"Core/EngineCore.h"
#include"Core/StringID.h"
#include<filesystem>

namespace Czuch
{
	class AssetsManager;

	enum CZUCH_API AssetType : uint8_t
	{
		TEXTURE = 0,
		MATERIAL = 1,
		MATERIAL_INSTANCE = 2,
		MESH = 3,
		SHADER = 4,
		ALL = 5
	};

	enum AssetInnerState
	{
		CREATED,
		LOADED,
		UNLOADED
	};

	enum AssetModeType
	{
		LOADED_TYPE,
		CREATED_TYPE
	};

	struct AssetHandle
	{
		Guid handle;
		bool IsValid() { return handle != Invalid_Handle_Id; }

		AssetHandle()
		{
			handle = Invalid_Handle_Id;
		}

		AssetHandle(Guid guid)
		{
			handle = guid;
		}

		bool operator==(const AssetHandle& other) const
		{
			return handle == other.handle;
		}

		static AssetHandle ToHandle(int handle)
		{
			return { handle };
		}
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

		const CzuchStr GetCountString() const
		{
			return std::to_string(counter);
		}
	};

	struct CZUCH_API ShortAssetInfo
	{
		const CzuchStr* name=nullptr;
		AssetType type= AssetType::ALL;
		AssetHandle asset;
		I32 resource=Invalid_Handle_Id;
		bool hiddenInEditor = false;
		ShortAssetInfo* next=nullptr;
	};

	class CZUCH_API Asset
	{
	public:
		Asset(const CzuchStr& resourcePath, const CzuchStr& resourceName,AssetsManager* assetsManager);
		Asset(const CzuchStr& resourceName, AssetsManager* assetsManager);
		virtual ~Asset() {}

		void SetPersistentStatus(bool isPersistent);

		const std::filesystem::path& GetRelativePath() const { return m_RelativePath; }
		const CzuchStr& AssetPath() const { return m_AssetPath; }
		const CzuchStr& AssetName() const { return m_AssetName; }
		const Guid GetGuid() const { return m_GUID.GetGuid(); }

		inline bool IsLoaded() { return m_State; }
		inline AssetHandle GetHandle() { return {m_GUID.GetGuid()}; }

		static CzuchStr GetNameFromPath(const CzuchStr& inStr);
		static CzuchStr GetTypeFromPath(const CzuchStr& inStr);

		virtual bool LoadAsset() {
			if (m_State == AssetInnerState::LOADED)
			{
				return true;
			}
			return false;
		}
		virtual bool UnloadAsset() = 0;
		virtual bool CreateFromData() = 0;
		virtual CzuchStr GetAssetLoadInfo() const = 0;
		virtual ShortAssetInfo* GetShortAssetInfo() = 0;

		inline void ForceUnload() { m_ForceUnload = true; }
		inline void IncrementRefCounter() { m_RefCounter.Up(); }
	protected:
		bool ShouldUnload();
	protected:
		AssetInnerState m_State;
		AssetModeType m_AssetType;
		AssetRefCounter m_RefCounter;
		ShortAssetInfo m_ShortInfo;
		AssetsManager* m_AssetsMgr;
	private:
		std::filesystem::path m_RelativePath;
		CzuchStr m_AssetPath;
		CzuchStr m_AssetName;
		StringID m_GUID;
	protected:
		bool m_ForceUnload;
		bool m_Persistent;
	};

}

