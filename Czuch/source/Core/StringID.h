#pragma once
#include"EngineCore.h"
#include<unordered_map>

namespace Czuch
{
	class CZUCH_API StringID
	{
	public:
		const CzuchStr& GetStrName() const;
		Guid GetGuid() const;
		U32 Compare(const StringID& comp);
		inline bool IsValid() const { return m_id != 0; }
		static StringID MakeStringID(const CzuchStr& rhl);
		static StringID MakeStringID(const CzuchStr&& rhl);
		StringID();
	private:
		StringID(Guid id,const CzuchStr& str);
	private:
		Guid m_id;
		CzuchStr m_str;
	private:
		static std::unordered_map<Guid, CzuchStr> s_NameLookup;
	};

#define sID(name)StringID::MakeStringID(name)

}
