#pragma once
#include <stdint.h>

class Error {
public:
	enum class Type {
		Connection = 0b11,
		Stream = 0b10
	};
	enum class Code : uint32_t
	{
		NO_ERROR,
		PROTOCOL_ERROR,
		INTERNAL_ERROR,
		FLOW_CONTROL_ERROR,
		SETTINGS_TIMEOUT,
		STREAM_CLOSED,
		FRAME_SIZE_ERROR,
		REFUSED_STREAM,
		CANCEL,
		COMPRESSION_ERROR,
		CONNECT_ERROR,
		ENHANCE_YOUR_CALM,
		INADEQUATE_SECURITY,
		HTTP_1_1_REQUIRED
	};
	Error(Type type, Code code) :  type(type), code(code) {

	}

	Error(Type type, Code code, std::string message) :  type(type), code(code), message(message) {

	}

	Type type;
	Code code;
	std::string message;
};