#include "HeaderImpl.h"
#include "../Header.h"
#include "Http.h"
#include <algorithm>

void Net::Http::V1::HeaderImpl::Encode(const Header* header, std::vector<uint8_t>::iterator &buffer) const {
	for (auto &entry : header->headerlist)
	{
		std::string line = Net::Http::V1::KeyUpper(entry.first) + ": " + entry.second + "\r\n";
		buffer = std::copy(line.begin(), line.end(), buffer);
	}
	const char nl[] = "\r\n";
	buffer = std::copy(nl, std::end(nl) - 1, buffer);
}

void Net::Http::V1::HeaderImpl::Decode(Header* header, std::vector<uint8_t>::const_iterator &buffer, const std::vector<uint8_t>::const_iterator &end) {
    const char sep[] = ": ";
	const char rn[] = "\r\n";
	while (buffer < end)
	{
		auto ofr = std::search(buffer, end, rn, std::end(rn));
		auto osep = std::search(buffer, ofr, sep, std::end(sep));
		if (osep != ofr)
		{
			std::string key(buffer, osep);
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			header->Add({ key, std::string(osep + 2, ofr) });
		} else {
			buffer = ofr + 2;
			return;
		}
		buffer = ofr + 2;
	}
}