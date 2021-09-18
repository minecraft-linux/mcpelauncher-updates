#pragma once
#include <vector>
#include <string>

namespace Net
{
	namespace Http
	{
        class Header;
        class HeaderImpl {
        public:
			virtual bool Add(Header* header, size_t hash, const std::pair<std::string, std::string> & pair);
			virtual void Encode(const Header* header, std::vector<uint8_t>::iterator & buffer) const;
			virtual void Decode(Header* header, std::vector<uint8_t>::const_iterator & buffer, const std::vector<uint8_t>::const_iterator & end);
        };
    }
}