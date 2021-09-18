#pragma once
#include "HeaderImpl.h"
#include <vector>
#include <memory>

namespace Net
{
	namespace Http
	{
		namespace V2 {
			class ResponseImpl : public HeaderImpl
			{
			public:
                ResponseImpl(const std::shared_ptr<HPack::Decoder> & decoder, const std::shared_ptr<HPack::Encoder> & encoder);
				void Encode(const Header * header, std::vector<uint8_t>::iterator & buffer) const override;
			};
		}
	}
}