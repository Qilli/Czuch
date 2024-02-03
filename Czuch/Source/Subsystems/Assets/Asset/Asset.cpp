#include "czpch.h"
#include "Asset.h"

namespace Czuch
{

	Asset::Asset(const CzuchStr& assetPath, const CzuchStr& assetName)
	{
		m_GUID = StringID::MakeStringID(assetPath);
		m_AssetName = assetName;
		m_AssetPath = assetPath;
		m_State = AssetInnerState::CREATED;
	}

	CzuchStr Asset::GetNameFromPath(const CzuchStr& inStr)
	{
		size_t result=inStr.find_last_of("/\\");
		CzuchStr str=inStr.substr(result + 1);
		return str;
	}

	CzuchStr Asset::GetTypeFromPath(const CzuchStr& inStr)
	{
		size_t result = inStr.find_last_of(".");
		CzuchStr str = inStr.substr(result + 1);
		return str;
	}

}
