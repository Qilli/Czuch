#pragma once
#include <string>
#include <vector>
#include <type_traits>
typedef unsigned long U64;
typedef unsigned int U32;
typedef int I32;
typedef float F32;
typedef double F64;
typedef unsigned short U16;
typedef short I16;
typedef unsigned char U8;
typedef char I8;
typedef I32 Guid;
typedef std::string CzuchStr;

const CzuchStr EMPTY_STRING = "";

namespace Czuch
{

    constexpr U32 crc32(const I8* str)
    {
        U32 crc = 0xFFFFFFFF;

        for (int i = 0; str[i] != '\0'; ++i)
        {
            crc ^= static_cast<U32>(str[i]);

            for (int j = 0; j < 8; ++j)
            {
                crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
            }
        }
        return crc ^ 0xFFFFFFFF;
    }

    constexpr U32 Hash(const CzuchStr& str)
	{
        return crc32(str.c_str());
	}

    constexpr U32 Hash(const I8* str)
    {
        return crc32(str);
    }

    template<typename E>
    struct enable_bitmask_operators {
        static constexpr bool enable = true;
    };

    template<typename E>
    constexpr typename std::enable_if<enable_bitmask_operators<E>::enable, E>::type operator&(E lhs, E rhs)
    {
        typedef typename std::underlying_type<E>::type underlying;
        return static_cast<E>(
            static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
    }

    template<typename E>
    constexpr bool HasFlag(E lhs, E rhs)
    {
        return (lhs & rhs) == rhs;
    }

    template<typename T>
    using Array = std::vector<T>;

}
