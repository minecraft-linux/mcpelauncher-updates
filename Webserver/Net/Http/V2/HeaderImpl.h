#pragma once
#include "../HeaderImpl.h"
#include "HPack/Decoder.h"
#include "HPack/Encoder.h"
namespace Net
{
	namespace Http
	{
        namespace V2 {
            class HeaderImpl : public virtual Http::HeaderImpl
            {
            public:
                std::shared_ptr<HPack::Decoder> decoder;
                std::shared_ptr<HPack::Encoder> encoder;
            public:
                HeaderImpl(const std::shared_ptr<HPack::Decoder> & decoder, const std::shared_ptr<HPack::Encoder> & encoder);
                virtual void Encode(const Header* header, std::vector<uint8_t>::iterator & buffer) const override;
                virtual void Decode(Header* header, std::vector<uint8_t>::const_iterator & buffer, const std::vector<uint8_t>::const_iterator & end) override;
            };
        }
    }
}