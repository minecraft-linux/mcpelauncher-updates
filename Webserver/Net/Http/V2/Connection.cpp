#include "Connection.h"
#include "Frame.h"
#include "RequestImpl.h"
#include "ResponseImpl.h"
#include <algorithm>

using namespace Net::Http::V2;

Connection::Connection(const std::shared_ptr<Stream>& stream, const std::shared_ptr<HPack::Decoder> &decoder, const std::shared_ptr<HPack::Encoder> &encoder) : stream(stream), request(std::make_shared<V2::RequestImpl>(decoder, encoder)), response(std::make_shared<V2::ResponseImpl>(decoder, encoder)) {

}

Net::Http::Request& Connection::GetRequest() {
	return request;
}
Net::Http::Response& Connection::GetResponse() {
	return response;
}

void Connection::SendResponse(bool endstream)
{
	stream->SendResponse(response, endstream);
}

void Connection::SendData(const uint8_t* buffer, int length, bool endstream)
{
	stream->SendData(buffer, length, endstream);
}