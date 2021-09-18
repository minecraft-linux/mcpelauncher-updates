#include "Request.h"
#include "../../utility.h"
#include <sstream>
#include <algorithm>

using namespace Net::Http;

Net::Http::Request::Request(const std::shared_ptr<Net::Http::HeaderImpl> &headerimpl) : Header(headerimpl) {

}

void Request::ParseUri(const std::string & uri)
{
	size_t sep = uri.find('?');
	if (sep != std::string::npos)
	{
		query = uri.substr(sep + 1);
		std::transform(query.begin(), query.end(), query.begin(), [](char ch) {return ch == '+' ? ' ' : ch; });
		query = Utility::UrlDecode(query);
		path = Utility::UrlDecode(uri.substr(0, sep));
		this->uri = path + '?' + query;
	}
	else
	{
		this->uri = path = Utility::UrlDecode(uri);
	}
}