#include "Frame.h"
#include "../../Socket.h"
#include <cstdio>
#include <cstdlib>

using namespace Net::Http::V2;

void AddUInt24(uint32_t number, std::vector<uint8_t>::iterator & destination)
{
	*destination++ = number >> 16;
	*destination++ = number >> 8;
	*destination++ = number;
}

void AddUInt32(uint32_t number, std::vector<uint8_t>::iterator & destination)
{
	*destination++ = number >> 24;
	*destination++ = number >> 16;
	*destination++ = number >> 8;
	*destination++ = number;
}

void AddUInt31(uint32_t number, std::vector<uint8_t>::iterator & destination)
{
	AddUInt32(number, destination);
	*(destination - 4) &= 0x7f;
}

Frame::Frame()
{
}

bool Frame::HasFlag(Flag flag) const
{
	return (uint8_t)flags & (uint8_t)flag;
}

std::vector<uint8_t> Frame::ToArray() const
{
	std::vector<uint8_t> frame(9);
	auto iter = frame.begin();
	AddUInt24(length, iter);
	*iter++ = (uint8_t)type;
	*iter++ = (uint8_t)flags;
	AddUInt31(stream->identifier, iter);
	return frame;
}