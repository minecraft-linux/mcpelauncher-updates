#pragma once
#include "HeaderImpl.h"

namespace Net
{
	namespace Http
	{
        namespace V2 {
            class RequestImpl : public HeaderImpl
            {
            public:
				using HeaderImpl::HeaderImpl;
			    virtual bool Add(Header* header, size_t hash, const std::pair<std::string, std::string> & pair) override;
            };
        }
	}
}