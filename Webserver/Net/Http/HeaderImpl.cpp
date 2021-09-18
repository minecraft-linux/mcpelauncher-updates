#include "HeaderImpl.h"
#include "Header.h"
#include <stdexcept>

size_t hscheme = Net::Http::Header::hash_fn(":scheme");
size_t hauthority = Net::Http::Header::hash_fn(":authority");
size_t hhost = Net::Http::Header::hash_fn("host");
size_t hcontenttype = Net::Http::Header::hash_fn("content-type");
size_t hcontentlength = Net::Http::Header::hash_fn("content-length");

bool Net::Http::HeaderImpl::Add(Header* header, size_t hash, const std::pair<std::string, std::string>& pair)
{
	if (hash == hscheme)
	{
		header->scheme = pair.second;
	}
	else if (hash == hauthority || hash == hhost)
	{
		header->authority = pair.second;
	}
	else if(hash == hcontenttype)
	{
		header->contenttype = pair.second;
	}
	else if (hash == hcontentlength)
	{
		header->contentlength = std::stoull(pair.second);
	}
	else
	{
		return false;
	}
	return true;
}

void Net::Http::HeaderImpl::Encode(const Header* header, std::vector<uint8_t>::iterator &buffer) const {
	throw std::runtime_error("Invalid Operation [Net::Http::HeaderImpl::Encode]");
}

void Net::Http::HeaderImpl::Decode(Header* header, std::vector<uint8_t>::const_iterator &buffer, const std::vector<uint8_t>::const_iterator &end) {
	throw std::runtime_error("Invalid Operation [Net::Http::HeaderImpl::Decode]");
}