#pragma once
#include <cstdint>

namespace Net {
	namespace Http {
		namespace V2 {
			template<class Iter>
			Iter GetUInt16(Iter buffer, uint16_t & number)
			{
				number = *buffer << 8 | *(buffer + 1);
				return buffer + 2;
			}

			template<class Iter>
			Iter GetUInt24(Iter buffer, uint32_t & number)
			{
				number = *buffer << 16 | *(buffer + 1) << 8 | *(buffer + 2);
				return buffer + 3;
			}

			template<class Iter>
			Iter GetUInt32(Iter buffer, uint32_t & number)
			{
				number = (*buffer << 24) | (*(buffer + 1) << 16) | (*(buffer + 2) << 8) | *(buffer + 3);
				return buffer + 4;
			}

			template<class Iter>
			Iter GetUInt31(Iter buffer, uint32_t & number)
			{
				auto pos = GetUInt32(buffer, number);
				number &= 0x7fffffff;
				return pos;
			}
        }
    }
}