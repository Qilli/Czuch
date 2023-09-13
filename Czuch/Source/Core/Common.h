#pragma once
#include"EngineCore.h"

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

	U32 Hash(const CzuchStr& str)
	{
        return crc32(str.c_str());
	}

    U32 Hash(const I8* str)
    {
        return crc32(str);
    }

}
