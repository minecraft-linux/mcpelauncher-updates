#include "Stream.h"
#include "Session.h"
#include "Setting.h"
#include "Connection.h"
#include "../Request.h"
#include "../Response.h"
#include <fstream>
#include <thread>
#include <filesystem>
#include <cstring>
#include "RequestImpl.h"

using namespace Net::Http::V2;
using namespace std::filesystem;

static std::unordered_map<Frame::Type, std::function<void(std::shared_ptr<Session>, std::shared_ptr<std::vector<uint8_t>>, std::shared_ptr<Frame>)>> framehandler = {
{ Frame::Type::HEADERS, [](std::shared_ptr<Session> session, std::shared_ptr<std::vector<uint8_t>> buffer, std::shared_ptr<Frame> frame) {
	if (frame->stream->identifier == 0)
		throw Error(Error::Type::Connection, Error::Code::PROTOCOL_ERROR, "HEADERS frames MUST be associated with a stream");
	auto stream = frame->stream;
	stream->state = frame->HasFlag(Frame::Flag::END_STREAM) ? Stream::State::half_closed_remote : Stream::State::open;
	auto beg = buffer->cbegin();
	auto end = beg + frame->length;	
	if (frame->HasFlag(Frame::Flag::PRIORITY))
	{
		beg = stream->ParsePriority(beg, *session);
		// beg = Stream::Priority::Parse(beg, stream->priority);
	}
	if (frame->HasFlag(Frame::Flag::PADDED))
	{
		uint8_t padlength = *beg++;
		if (padlength > frame->length)
			throw Error::Code::PROTOCOL_ERROR;
		end -= padlength;
		if((end - beg) < padlength)
			throw Error(Error::Type::Connection, Error::Code::PROTOCOL_ERROR, "Padding exceeds the size remaining for the header block");
	}
	auto con = std::make_shared<Connection>(stream, session->GetDecoder(), session->GetEncoder());
	con->GetRequest().Decode(beg, end);
	if (frame->HasFlag(Frame::Flag::END_HEADERS))
	{
		session->requesthandler(con);
	}
} },{ Frame::Type::PRIORITY, [](std::shared_ptr<Session> session, std::shared_ptr<std::vector<uint8_t>> buffer, std::shared_ptr<Frame> frame) {
	if(frame->length != 5)
		throw Error(Error::Type::Stream, Error::Code::FRAME_SIZE_ERROR);
		frame->stream->ParsePriority(buffer->cbegin(), *session);
	// Stream::Priority::Parse(buffer->cbegin(), frame->stream->priority);
} },{ Frame::Type::RST_STREAM, [](std::shared_ptr<Session> session, std::shared_ptr<std::vector<uint8_t>> buffer, std::shared_ptr<Frame> frame) {
	if (frame->stream->identifier == 0)
		throw Error(Error::Type::Connection, Error::Code::PROTOCOL_ERROR, "RST_STREAM frames MUST be associated with a stream");
	if (frame->stream->state == Stream::State::idle)
		throw Error(Error::Type::Connection, Error::Code::PROTOCOL_ERROR, "RST_STREAM frames MUST NOT be sent for a stream in the \"idle\" state");
	if(frame->length != 4)
		throw Error(Error::Type::Stream, Error::Code::FRAME_SIZE_ERROR);
	Error::Code code;
	GetUInt32(buffer->cbegin(), (uint32_t&)code);
    frame->stream->Reset(code);
} },{ Frame::Type::SETTINGS, [](std::shared_ptr<Session> session, std::shared_ptr<std::vector<uint8_t>> buffer, std::shared_ptr<Frame> frame) {
	if (frame->stream->identifier != 0)
		throw Error(Error::Type::Connection, Error::Code::PROTOCOL_ERROR, "SETTINGS frames always apply to a connection, never a single stream");
	if (!frame->HasFlag(Frame::Flag::ACK))
	{
		if((frame->length % 6) != 0)
			throw Error(Error::Type::Connection, Error::Code::FRAME_SIZE_ERROR);
		auto pos = buffer->cbegin(), end = pos + frame->length;
		while (pos != end)
		{
			Setting s;
			pos = GetUInt16(pos, (uint16_t&)s);
			((uint16_t&)s)--;
			pos = GetUInt32(pos, session->settings[s]);
			// session->GetSetting(s) = GetUInt32(pos);
		}
		{
			Frame response;
			response.length = 0;
			response.type = Frame::Type::SETTINGS;
			response.flags = Frame::Flag::ACK;
			response.stream = frame->stream;
			frame->stream->SendFrame(response);
		}
	} else {
		if(frame->length != 0)
			throw Error(Error::Type::Connection, Error::Code::FRAME_SIZE_ERROR);
	}
} },{ Frame::Type::PING, [](std::shared_ptr<Session> session, std::shared_ptr<std::vector<uint8_t>> buffer, std::shared_ptr<Frame> frame) {
	if (frame->stream->identifier != 0)
		throw Error(Error::Type::Connection, Error::Code::PROTOCOL_ERROR, "PING frames are not associated with any individual stream");
	if(frame->length != 8)
		throw Error(Error::Type::Stream, Error::Code::FRAME_SIZE_ERROR);
	if (!frame->HasFlag(Frame::Flag::ACK))
	{
		Frame response;
		response.length = frame->length;
		response.type = Frame::Type::PING;
		response.flags = Frame::Flag::ACK;
		response.stream = frame->stream;
    	auto pos = buffer->begin();
		frame->stream->SendFrame(response, pos);
	}
} },{ Frame::Type::GOAWAY, [](std::shared_ptr<Session> session, std::shared_ptr<std::vector<uint8_t>> buffer, std::shared_ptr<Frame> frame) {
	if (frame->stream->identifier != 0)
		throw Error(Error::Type::Connection, Error::Code::PROTOCOL_ERROR, "The GOAWAY frame applies to the connection, not a specific stream");
	auto pos = buffer->cbegin();
	uint32_t laststreamid;
	pos = GetUInt31(pos, laststreamid);
	Error::Code code;
	pos = GetUInt32(pos, (uint32_t&)code);
	{
		Frame response;
		response.length = frame->length;
		response.type = Frame::Type::GOAWAY;
		response.flags = (Frame::Flag)0;
		response.stream = frame->stream; 
		auto wpos = buffer->begin();
		AddUInt31(laststreamid, wpos);
		AddUInt32((uint32_t)Error::Code::NO_ERROR, wpos);
        auto pos = buffer->begin();
		frame->stream->SendFrame(response, pos);
	}
} },{ Frame::Type::WINDOW_UPDATE, [](std::shared_ptr<Session> session, std::shared_ptr<std::vector<uint8_t>> buffer, std::shared_ptr<Frame> frame) {
	uint32_t windowinc;
	GetUInt31(buffer->cbegin(), windowinc);
	frame->stream->rwindowsize += windowinc;
} }
};

void Net::Http::V2::Session::Start()
{
	std::vector<uint8_t> buffer(settings[Setting::MAX_FRAME_SIZE]);
	{
		uint8_t preface[] = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
		// Frame frame;
		// frame.flags = Frame::Flag(0);
		// frame.type = Frame::Type::SETTINGS;
		// frame.length = 0;
		// socket->SendAll(preface, sizeof(preface) - 1);
		auto in = socket->GetInputStream();
		if (!in.ReceiveAll(buffer.data(), sizeof(preface) - 1) || memcmp(buffer.data(), preface, sizeof(preface) - 1))
			throw std::runtime_error(u8"Invalid Connection Preface");
	}
	while (true)
	{
		auto is = socket->GetInputStream();
		if (!is.ReceiveAll(buffer.data(), 9))
			throw Error(Error::Type::Connection, Error::Code::FRAME_SIZE_ERROR);
		auto pos = buffer.cbegin();
		auto frame = std::make_shared<Frame>();
		pos = GetUInt24(pos, frame->length);			
		frame->type = (Frame::Type)*pos++;
		frame->flags = (Frame::Flag)*pos++;
		uint32_t streamid;
		pos = GetUInt32(pos, streamid);
		frame->stream = GetStream(streamid);
		if(!frame->stream) {
			frame->stream = std::make_shared<Stream>(socket, streamid, settings[Setting::INITIAL_WINDOW_SIZE]);
			streams.emplace_back(frame->stream);
		}
		if (buffer.size() < frame->length)
		{
			if (frame->length > settings[Setting::MAX_FRAME_SIZE])
				throw Error(frame->type == Frame::Type::HEADERS || frame->type == Frame::Type::PUSH_PROMISE || frame->type == Frame::Type::CONTINUATION || frame->stream->identifier == 0 ? Error::Type::Connection : Error::Type::Stream, Error::Code::FRAME_SIZE_ERROR);
			buffer.resize(settings[Setting::MAX_FRAME_SIZE]);
		}
		if(frame->type == Frame::Type::DATA) {
			if(!frame->stream) {
				continue;
			}
			if (frame->stream->identifier == 0)
				throw Error(Error::Type::Connection, Error::Code::PROTOCOL_ERROR, "DATA frames MUST be associated with a stream");
			if (!(((uint8_t)frame->stream->state & (uint8_t)Stream::State::half_closed_local) == (uint8_t)Stream::State::half_closed_local))
				throw Error(Error::Type::Stream, Error::Code::STREAM_CLOSED);
			auto beg = buffer.cbegin();
			auto end = beg + frame->length;
			if (frame->HasFlag(Frame::Flag::PADDED))
			{
				uint8_t padlength = *beg++;
				if (padlength > frame->length)
					throw Error(Error::Type::Connection, Error::Code::PROTOCOL_ERROR, "The length of the padding is the length of the frame payload or greater");
				end -= padlength;
			}
			throw std::runtime_error("Not Implemented");
			//frame->stream->ReceiveData(Frame)
		}
		else {
			if (!is.ReceiveAll(buffer.data(), frame->length))
				throw Error(Error::Type::Connection, Error::Code::FRAME_SIZE_ERROR);

			// auto that = std::shared_ptr<Session>(this, weak_from_this());
			framehandler.at(frame->type)(std::static_pointer_cast<Net::Http::V2::Session>(shared_from_this()), std::make_shared<std::vector<uint8_t>>(buffer), frame);
		}
	}
}

std::shared_ptr<Stream> Net::Http::V2::Session::GetStream(uint32_t id) const {
	auto res = std::find_if(streams.begin(), streams.end(), [id](const std::shared_ptr<Stream>& stream) {
		return stream->identifier == id;
	});
	if(res != streams.end()) {
		return *res;
	}
	return std::shared_ptr<Stream>();
}