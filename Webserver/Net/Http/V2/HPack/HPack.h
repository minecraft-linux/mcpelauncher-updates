#pragma once
#include <cstdint>
#include <utility>
#include <string>
#include <string_view>

#ifndef IMPORT
#ifdef _WIN32
#define IMPORT  __declspec( dllimport )
#else
#define IMPORT
#endif
#endif

namespace std {
template<class ...T> inline bool operator==(const std::basic_string_view<T...> & l, const std::basic_string<T...> & r) {
	return l == std::basic_string<T...>(r.data(), r.length());
}
template<class L1, class L2, class R1, class R2> inline bool operator==(const std::pair<L1, R1> & l, const std::pair<L2, R2> & r) {
	return l.first == r.first && l.second == r.second;
}

// inline bool operator==(const std::pair<std::string_view, std::string_view> & l, const std::pair<std::string, std::string> & r) {
// 	// return true;
// 	// return l.first == r.first && l.second == r.second;
// 	// return operator==<std::string_view, std::string, std::string_view, std::string>(l, r);
// }
}

namespace Net
{
	namespace Http
	{
		namespace V2
		{
			namespace HPack
			{
				IMPORT extern const std::pair<std::string_view, std::string_view> StaticTable[61];
				IMPORT extern const std::pair<uint32_t, uint8_t> StaticHuffmanTable[257];
			}
		}
	}
}
