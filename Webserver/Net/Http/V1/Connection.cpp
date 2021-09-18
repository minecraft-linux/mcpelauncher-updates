#include "Connection.h"
#include "RequestImpl.h"
#include "ResponseImpl.h"

using namespace Net::Http::V1;

Net::Http::V1::Connection::Connection(const std::shared_ptr<Net::Socket> &socket) : socket(socket), request(std::make_shared<RequestImpl>()), response(std::make_shared<ResponseImpl>()) {

}

Net::Http::Request &Net::Http::V1::Connection::GetRequest() {
	return request;
}

Net::Http::Response &Net::Http::V1::Connection::GetResponse() {
	return response;
}

void Connection::SendResponse(bool endstream)
{
	std::vector<uint8_t> buffer(1 << 20);
	auto end = buffer.begin();
	response.Encode(end);
	auto os = socket->GetOutputStream();
	os.SendAll(buffer.data(), end - buffer.begin());
}

void Connection::SendData(const uint8_t * buffer, int length, bool endstream)
{
	auto os = socket->GetOutputStream();
	os.SendAll(buffer, length);
}