#include "RequestImpl.h"
#include "../Request.h"
#include "../Header.h"
#include <algorithm>

void Net::Http::V1::RequestImpl::Decode(Header * header, std::vector<uint8_t>::const_iterator & buffer, const std::vector<uint8_t>::const_iterator & end) {
    std::vector<uint8_t>::const_iterator ofr = std::find(buffer, end, ' ');
	((Request*)header)->method = std::string(buffer, ofr);
	buffer = ofr + 1;
	ofr = std::find(buffer, end, ' ');
	((Request*)header)->ParseUri(std::string(buffer, ofr));
	buffer = ofr + 1;
	const char rn[] = "\r\n";
	buffer = std::search(buffer, end, rn, std::end(rn)) + 2;
	HeaderImpl::Decode(header, buffer, end);
}