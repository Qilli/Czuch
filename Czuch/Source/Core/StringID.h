#pragma once
#include"EngineCore.h"

namespace Czuch
{
	class CZUCH_API StringID
	{
	public:
		const CzuchStr& GetStrName() const;
		ID GetID() const;
		U32 Compare(const StringID& comp);
		static StringID MakeStringID(const CzuchStr& rhl);
	private:
		StringID(ID id,const CzuchStr& str);
		StringID();
	private:
		ID m_id;
		CzuchStr m_str;
	private:
		static std::unordered_map<ID, CzuchStr> s_NameLookup;
	};

#define sID(name)StringID::MakeStringID(name)
}
