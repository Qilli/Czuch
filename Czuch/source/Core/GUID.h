#pragma once

#include<xhash>
namespace Czuch
{
	class CZUCH_API GUID
	{
	public:
		GUID();
		GUID(uint64_t guid);
		GUID(const GUID& other)=default;
		GUID& operator=(const GUID& other)=default;
		operator uint64_t() const { return m_GUID; }
		uint64_t AsUint64() const { return m_GUID; }
		std::string ToString() const{ return std::to_string(m_GUID); }
		bool IsValid() const { return m_GUID != 0; }
	private:
		uint64_t m_GUID;
	};

	#define INVALID_GUID GUID(0);
}
namespace std
{
	template<>
	struct hash<Czuch::GUID>
	{
		size_t operator()(const Czuch::GUID& guid) const
		{
			return hash<uint64_t>()((uint64_t)guid);
		}
	};
}


