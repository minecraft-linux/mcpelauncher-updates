#include "ResponseImpl.h"
#include "../Response.h"

using namespace Net::Http::V2;

ResponseImpl::ResponseImpl(const std::shared_ptr<HPack::Decoder> & decoder, const std::shared_ptr<HPack::Encoder> & encoder) : HeaderImpl(decoder, encoder) {
	
}


void ResponseImpl::Encode(const Net::Http::Header * header, std::vector<uint8_t>::iterator & buffer) const
{
	std::vector<std::pair<std::string, std::string>> headerlist = { { ":status", std::to_string(((Response*)header)->status) } };
	encoder->AddHeaderBlock(buffer, headerlist);
	HeaderImpl::Encode(header, buffer);
}