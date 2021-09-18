#include "RequestImpl.h"
#include "../Request.h"
#include "../Header.h"
#include <algorithm>

size_t hmethod = Net::Http::Header::hash_fn(":method");
size_t hpath = Net::Http::Header::hash_fn(":path");

bool Net::Http::V2::RequestImpl::Add(Header* header, size_t hash, const std::pair<std::string, std::string> &pair) {

	if (hash == hmethod)
	{
		((Request*)header)->method = pair.second;
	}
	else if (hash == hpath)
	{
		((Request*) header)->ParseUri(pair.second);
	}
	else
	{
		return false;
	}
	return true;
}