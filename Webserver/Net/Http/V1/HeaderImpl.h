#pragma once
#include "../HeaderImpl.h"
namespace Net
{
	namespace Http
	{
        namespace V1 {
            class HeaderImpl : public virtual Http::HeaderImpl
            {
            public:
                virtual void Encode(const Header* header, std::vector<uint8_t>::iterator & buffer) const override;
                virtual void Decode(Header* header, std::vector<uint8_t>::const_iterator & buffer, const std::vector<uint8_t>::const_iterator & end) override;
            };
        }
    }
}