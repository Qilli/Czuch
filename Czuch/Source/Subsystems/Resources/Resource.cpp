#include "czpch.h"
#include "Resource.h"

namespace Czuch
{

	Resource::Resource(const CzuchStr& resourcePath, const CzuchStr& resourceName)
	{
		m_GUID = StringID::MakeStringID(resourcePath);
		m_ResourceName = resourceName;
		m_ResourcePath = resourcePath;
		m_State = ResourceInnerState::CREATED;
	}

	CzuchStr Resource::GetNameFromPath(const CzuchStr& inStr)
	{
		size_t result=inStr.find_last_of("/\\");
		CzuchStr str=inStr.substr(result + 1);
		return str;
	}

	CzuchStr Resource::GetTypeFromPath(const CzuchStr& inStr)
	{
		size_t result = inStr.find_last_of(".");
		CzuchStr str = inStr.substr(result + 1);
		return str;
	}

}
