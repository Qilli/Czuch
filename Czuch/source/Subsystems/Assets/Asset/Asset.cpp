#include "czpch.h"
#include "Asset.h"
#include "../AssetsManager.h"

namespace Czuch
{

	Asset::Asset(const CzuchStr& assetPath, const CzuchStr& assetName, AssetsManager* assetsManager)
	{
		m_AssetsMgr = assetsManager;
		m_GUID = StringID::MakeStringID(assetPath);
		m_AssetName = assetName;
		m_AssetPath = AssetsManager::GetStartPath() + assetPath;
		m_RelativePath = assetPath;
		m_State = AssetInnerState::CREATED;
		m_ForceUnload = false;
	}

	Asset::Asset(const CzuchStr& resourceName, AssetsManager* assetsManager):Asset(resourceName,resourceName,assetsManager)
	{
		m_ForceUnload = false;
	}

	void Asset::SetPersistentStatus(bool isPersistent)
	{
		m_Persistent = isPersistent;
	}

	CzuchStr Asset::GetNameFromPath(const CzuchStr& inStr)
	{
		size_t result=inStr.find_last_of("//");
		CzuchStr str=inStr.substr(result + 1);
		return str;
	}

	CzuchStr Asset::GetTypeFromPath(const CzuchStr& inStr)
	{
		size_t result = inStr.find_last_of(".");
		CzuchStr str = inStr.substr(result + 1);
		return str;
	}

	bool Asset::ShouldUnload()
	{
		return (m_State == AssetInnerState::LOADED && ((!m_RefCounter.Down() && !m_Persistent) || m_ForceUnload));
	}

}
