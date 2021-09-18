#include "ResponseImpl.h"
#include "../Response.h"
#include <algorithm>

using namespace Net::Http::V1;

void ResponseImpl::Encode(const Net::Http::Header * header, std::vector<uint8_t>::iterator & buffer) const
{
	std::string statusline = "HTTP/1.1 " + std::to_string(((Response*)header)->status) + "\r\n";
	buffer = std::copy(statusline.begin(), statusline.end(), buffer);
	HeaderImpl::Encode(header, buffer);
}

void ResponseImpl::Decode(Net::Http::Header *header, std::vector<uint8_t>::const_iterator &buffer, const std::vector<uint8_t>::const_iterator &end) {
	std::vector<uint8_t>::const_iterator ofr = std::find(buffer, end, ' ');
	//((Response*)header)->protocol = std::string(buffer, ofr);
	buffer = ofr + 1;
	ofr = std::find(buffer, end, ' ');
	((Response*)header)->status = std::stoi(std::string(buffer, ofr));
	buffer = ofr + 1;
	const char rn[] = "\r\n\r\n";
	buffer = std::search(buffer, end, rn, rn + 2) + 2;
	auto hend = std::search(buffer, end, rn, rn + 4) + 2;
	HeaderImpl::Decode(header, buffer, end);
	buffer = hend + 2;
}