#include <string>

namespace Utility {
	std::string Replace(const std::string &str, const char * search, const char * replace);

	std::string UrlEncode(const std::string &url);
	std::string UrlDecode(const std::string &encoded);

	std::string CharToHex(unsigned char c);
	unsigned char HexToChar(const std::string &str);
}