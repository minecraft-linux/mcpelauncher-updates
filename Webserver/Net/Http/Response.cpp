#include "Response.h"

using namespace Net::Http;

size_t hstatus = Net::Http::Header::hash_fn(":status");

Net::Http::Response::Response(const std::shared_ptr<HeaderImpl>& headerimpl) : Header(headerimpl), status(200)
{
}

bool Net::Http::Response::Add(size_t hash, const std::pair<std::string, std::string>& pair)
{
	if (hash == hstatus)
	{
		status = std::stol(pair.second);
	}
	else
	{
		return Header::Add(hash, pair);
	}
	return true;
}