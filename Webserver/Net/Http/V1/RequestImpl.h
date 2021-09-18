#pragma once
#include "HeaderImpl.h"

namespace Net
{
	namespace Http
	{
        namespace V1 {
            class RequestImpl : public HeaderImpl
            {
            public:
                virtual void Decode(Header* header, std::vector<uint8_t>::const_iterator & buffer, const std::vector<uint8_t>::const_iterator & end) override;
            };
        }
	}
}