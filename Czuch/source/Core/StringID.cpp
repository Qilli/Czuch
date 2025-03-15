#include"czpch.h"
#include"StringID.h"
#include"Common.h"


namespace Czuch
{
	std::unordered_map<Guid, CzuchStr> StringID::s_NameLookup;

	const CzuchStr& StringID::GetStrName() const
	{
		return m_str;
	}

	Guid StringID::GetGuid() const
	{
		return m_id;
	}

	U32 StringID::Compare(const StringID& comp)
	{
		return comp.m_id == m_id ? 0 : (m_id>comp.m_id?-1:1);
	}

	StringID::StringID(Guid id,const CzuchStr& str)
	{
		m_id = id;
		m_str = str;
	}

	StringID::StringID()
	{
		m_id = 0;
	}

	StringID StringID::MakeStringID(const CzuchStr& rhl)
	{
		U32 hashValue = Hash(rhl);
		auto val = StringID::s_NameLookup.find(hashValue);
		if (val != StringID::s_NameLookup.end())
		{
			return StringID(val->first, val->second);
		}

		StringID::s_NameLookup.insert({ hashValue, rhl });
		return StringID(hashValue, rhl);
	}

	StringID StringID::MakeStringID(const CzuchStr&& rhl)
	{
		U32 hashValue = Hash(rhl);
		auto val = StringID::s_NameLookup.find(hashValue);
		if (val != StringID::s_NameLookup.end())
		{
			return StringID(val->first, val->second);
		}

		StringID::s_NameLookup.insert({ hashValue, rhl });
		return StringID(hashValue, rhl);
	}
}