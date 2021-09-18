#include "HeaderImpl.h"
#include "../Header.h"
#include <algorithm>

Net::Http::V2::HeaderImpl::HeaderImpl(const std::shared_ptr<HPack::Decoder> &decoder, const std::shared_ptr<HPack::Encoder> &encoder) {
	this->decoder = decoder;
	this->encoder = encoder;
}

void Net::Http::V2::HeaderImpl::Encode(const Header* header, std::vector<uint8_t>::iterator &buffer) const {
	std::vector<std::pair<std::string, std::string>> el;
	if(!header->scheme.empty())
		el.push_back({ ":scheme", header->scheme });
	if (!header->authority.empty())
		el.push_back({ ":authority", header->authority });
	if (!header->contenttype.empty())
		el.push_back({ "content-type", header->contenttype });
	if(header->contentlength != 0)
		el.push_back({ "content-length", std::to_string(header->contentlength) });
	encoder->AddHeaderBlock(buffer, el);
	encoder->AddHeaderBlock(buffer, header->headerlist);
}

void Net::Http::V2::HeaderImpl::Decode(Header* header, std::vector<uint8_t>::const_iterator &buffer, const std::vector<uint8_t>::const_iterator &end) {
	decoder->DecodeHeaderblock(header, buffer, end);
}