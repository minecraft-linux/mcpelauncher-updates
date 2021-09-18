#pragma once
#include <cstdint>

enum class Setting : uint16_t
{
	HEADER_TABLE_SIZE,
	ENABLE_PUSH,
	MAX_CONCURRENT_STREAMS,
	INITIAL_WINDOW_SIZE,
	MAX_FRAME_SIZE,
	MAX_HEADER_LIST_SIZE
};