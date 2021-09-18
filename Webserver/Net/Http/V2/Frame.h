#pragma once
#include <cstdint>
#include <vector>
#include "Stream.h"

void AddUInt31(uint32_t number, std::vector<uint8_t>::iterator & destination);
void AddUInt32(uint32_t number, std::vector<uint8_t>::iterator & destination);

namespace Net
{
	namespace Http
	{
		namespace V2
		{
			struct Frame
			{
			public:
				enum class Type : uint8_t
				{
					DATA = 0x0,
					HEADERS = 0x1,
					PRIORITY = 0x2,
					RST_STREAM = 0x3,
					SETTINGS = 0x4,
					PUSH_PROMISE = 0x5,
					PING = 0x6,
					GOAWAY = 0x7,
					WINDOW_UPDATE = 0x8,
					CONTINUATION = 0x9
				};

				enum class Flag : uint8_t
				{
					END_STREAM = 0x1,
					ACK = 0x1,
					END_HEADERS = 0x4,
					PADDED = 0x8,
					PRIORITY = 0x20
				};

				uint32_t length;
				Type type;
				Flag flags;
				std::shared_ptr<Stream> stream;
				Frame();
				bool HasFlag(Flag flag) const;
				std::vector<uint8_t> ToArray() const;
			};
		}
	}
}