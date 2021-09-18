#include "Stream.h"
#include "Frame.h"

using namespace Net::Http::V2;

Stream::Stream(const std::shared_ptr<Net::Socket> & socket, uint32_t identifier, uint32_t initialwindowsize)
{
	this->socket = socket;
	this->identifier = identifier;
	this->rwindowsize = initialwindowsize;
	this->hwindowsize = initialwindowsize;
	exclusive = false;
	weight = 16;
	state = State::idle;
}

void Stream::Reset(Error::Code code)
{
	// Frame frame;
	// frame.type = Frame::Type::RST_STREAM;
	// frame.stream = shared_from_this();
	// frame.flags = Frame::Flag(0);
	// frame.length = 4;
	throw std::runtime_error("Not Implemented");
}

void Stream::SendFrame(const Frame &frame)
{
	auto os = socket->GetOutputStream();
	os.SendAll(frame.ToArray());
}

void Stream::SendFrame(const Frame &frame, std::vector<uint8_t>::iterator & data)
{
	auto os = socket->GetOutputStream();
	os.SendAll(frame.ToArray());
	os.SendAll(&*data, frame.length);
}

void Stream::SendResponse(const Net::Http::Response & response, bool endstream)
{
	std::vector<uint8_t> buffer(1 << 20);
	auto end = buffer.begin();
	response.Encode(end);
	Frame result;
	result.length = (uint32_t)(end - buffer.begin());
	result.type = Frame::Type::HEADERS;
	result.flags = Frame::Flag::END_HEADERS;
	if(endstream) 
		(uint8_t&)result.flags |= (uint8_t)Frame::Flag::END_STREAM;
	result.stream = shared_from_this();
        auto pos = buffer.begin();
	SendFrame(result, pos);
}

void Stream::SendData(const uint8_t * buffer, int length, bool endstream)
{
	Frame result;
	result.type = Frame::Type::DATA;
	result.flags = (Frame::Flag)0;
	result.stream = shared_from_this();
	uint32_t chunk = 8192;//settings[Setting::MAX_FRAME_SIZE];
	do
	{
		result.length = std::min<uint32_t>(chunk, length);
		if (endstream && result.length == length)
			result.flags = Frame::Flag::END_STREAM;
		{
			auto os = socket->GetOutputStream();
			os.SendAll(result.ToArray());
			os.SendAll(buffer, result.length);
		}
		length -= result.length;
		buffer += result.length;
	} while (length > 0);
}

void Stream::ReceiveData(int length, bool endstream) {
	if(hwindowsize > window.size()) window.resize(hwindowsize);
	hwindowsize -= length;
}