#ifndef NET_HTTPV2_STREAM_H1
#define NET_HTTPV2_STREAM_H1

#include "../Request.h"
#include "../Response.h"
#include "ErrorCode.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <functional>
#include <condition_variable>

namespace Net
{
	class Socket;
	namespace Http
	{
		namespace V2
		{
			struct Frame;
			class Session;
			class Stream : public std::enable_shared_from_this<Stream>
			{
			private:
				std::shared_ptr<Net::Socket> socket;
				std::function<void(std::vector<uint8_t>::const_iterator & buffer, uint32_t length)> _ondata;
				std::function<void(Frame & frame, std::vector<uint8_t>::const_iterator & buffer, uint32_t length)> _oncontinuation;
				std::condition_variable cond_var;
				std::condition_variable data;
				std::condition_variable continuation;
				std::vector<uint8_t> window;
				std::vector<uint8_t>::const_iterator windowbeg;
				std::vector<uint8_t>::const_iterator windowend;
			public:
				enum class State : uint8_t
				{
					idle = 0b0,
					reserved_local = 0b0100,
					reserved_remote = 0b1000,
					open = 0b11,
					half_closed_local = 0b10,
					half_closed_remote = 0b01,
					closed = 0b10000
				};
				Stream(const std::shared_ptr<Socket> & socket, uint32_t identifier, uint32_t initialwindowsize);
				uint32_t identifier;
				State state;
				std::shared_ptr<Stream> dependency;
				std::vector<std::shared_ptr<Stream>> children;
				bool exclusive;
				uint8_t weight;
				uint32_t rwindowsize;
				uint32_t hwindowsize;
				void Reset(Error::Code code);
				void SendFrame(const Frame & frame);
				void SendFrame(const Frame & frame, std::vector<uint8_t>::iterator & data);
				void SendResponse(const Net::Http::Response & response, bool endstream);
				void SendData(const uint8_t* buffer, int length, bool endstream);
				void ReceiveData(int length, bool endstream);
				template <class Iter>
				Iter ParsePriority(Iter pos, const Session & session);
			};
		}
	}
}
#endif

#ifndef NET_HTTPV2_STREAM_H2
#define NET_HTTPV2_STREAM_H2
#include "Support.h"
#include "Session.h"

template<class Iter> Iter Net::Http::V2::Stream::ParsePriority(Iter pos, const Net::Http::V2::Session &session) {
	exclusive = *pos & 0x80;
	uint32_t dependency;
	pos = GetUInt31(pos, dependency);
	this->dependency = session.GetStream(dependency);
	if(exclusive) {
		children = this->dependency->children;
		this->dependency->children = {  };
	}
	weight = *pos++;
	return pos;
}
#endif

#include "../../Socket.h"
#include "Frame.h"