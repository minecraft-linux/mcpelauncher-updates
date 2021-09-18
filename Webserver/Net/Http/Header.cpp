#include "Header.h"
#include "V1/Http.h"

std::hash<std::string> Net::Http::Header::hash_fn;

Net::Http::Header::Header(const std::shared_ptr<Net::Http::HeaderImpl> &headerimpl) : headerimpl(headerimpl), contentlength(0) {

}

void Net::Http::Header::Add(const std::pair<std::string, std::string>& pair)
{
	if (!Add(hash_fn(pair.first), pair))
	{
		headerlist.insert(pair);
	}
}

bool Net::Http::Header::Add(size_t hash, const std::pair<std::string, std::string>& pair)
{
	return headerimpl->Add(this, hash, pair);
}

void Net::Http::Header::Encode(std::vector<uint8_t>::iterator &buffer) const {
	headerimpl->Encode(this, buffer);
}

void Net::Http::Header::Decode(std::vector<uint8_t>::const_iterator & buffer, const std::vector<uint8_t>::const_iterator & end) {
	headerimpl->Decode(this, buffer, end);
}